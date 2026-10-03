// Copyright 2024 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "yaets/tracing.hpp"
#include "gtest/gtest.h"
#include <sys/stat.h>

#include <atomic>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

class TraceGuardTest : public yaets::TraceGuard
{
public:
  TraceGuardTest(yaets::TraceSession & session, const std::string & trace_name)
  : TraceGuard(session, trace_name) {}

  std::string extract_trace_name_test(const std::string & function_signature)
  {
    return extract_trace_name(function_signature);
  }
};

TEST(yaets, TraceSessionInitStop) {
  yaets::TraceSession trace_session("test_trace.log");
  std::ofstream file("test_trace.log");
  ASSERT_TRUE(file.is_open());
  file.close();
  trace_session.stop();
  std::remove("test_trace.log");
}

TEST(yaets, TraceSessionWriteEvent) {
  yaets::TraceSession trace_session("test_trace_event.log");
  trace_session.register_trace(
    "testFunction", std::chrono::nanoseconds(100),
    std::chrono::nanoseconds(200));
  trace_session.stop();
  std::ifstream file("test_trace_event.log");
  ASSERT_TRUE(file.is_open());
  std::string line;
  ASSERT_TRUE(std::getline(file, line));
  EXPECT_FALSE(line.empty());
  file.close();
  std::remove("test_trace_event.log");
}

TEST(yaets, TraceGuardNameExtraction) {
  yaets::TraceSession trace_session("no_file.log");
  TraceGuardTest trace_guard(trace_session, "no_file");
  ASSERT_EQ(trace_guard.extract_trace_name_test("function_1(std::string args)"), "function_1");
  ASSERT_EQ(trace_guard.extract_trace_name_test("function_1()"), "function_1");
  trace_session.stop();
  std::remove("no_file.log");
}

TEST(yaets, TraceGuardStartEndTimes) {
  yaets::TraceSession trace_session("test_guard_times.log");
  {
    yaets::TraceGuard trace_guard(trace_session, "testFunction");
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    ASSERT_LE(trace_guard.get_start_time().count(), now.count());
  }
  trace_session.stop();
  std::remove("test_guard_times.log");
}

TEST(yaets, NamedSharedTraceBasic) {
  yaets::TraceSession trace_session("test_named_trace.log");
  yaets::NamedSharedTrace shared_trace(trace_session, "sharedFunction");
  shared_trace.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  shared_trace.end();
  trace_session.stop();
  std::remove("test_named_trace.log");
}

TEST(yaets, NamedSharedTraceOverCapacity) {
  yaets::TraceSession trace_session("test_over_capacity.log");
  yaets::NamedSharedTrace shared_trace(trace_session, "overCapacityFunction");
  for (size_t i = 0; i < shared_trace.TRACE_SIZE_INIT + 5; ++i) {
    shared_trace.start();
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    shared_trace.end();
  }
  trace_session.stop();
  std::ifstream file("test_over_capacity.log");
  ASSERT_TRUE(file.is_open());
  std::string line;
  ASSERT_TRUE(std::getline(file, line));
  EXPECT_FALSE(line.empty());
  file.close();
  std::remove("test_over_capacity.log");
}

TEST(yaets, TraceRegistrySingleTrace) {
  yaets::TraceSession session("test_trace_registry.log");
  yaets::TraceRegistry & registry = yaets::TraceRegistry::getInstance();
  registry.registerTrace("trace1", session);
  registry.startTrace("trace1");
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  registry.endTrace("trace1");
  session.stop();
  std::ifstream file("test_trace_registry.log");
  ASSERT_TRUE(file.is_open());
  std::string line;
  ASSERT_TRUE(std::getline(file, line));
  EXPECT_FALSE(line.empty());
  file.close();
  std::remove("test_trace_registry.log");
}

TEST(yaets, TraceRegistryMultipleTraces) {
  yaets::TraceSession session("test_multiple_traces.log");
  yaets::TraceRegistry & registry = yaets::TraceRegistry::getInstance();
  registry.registerTrace("trace1", session);
  registry.registerTrace("trace2", session);
  registry.startTrace("trace1");
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  registry.endTrace("trace1");
  registry.startTrace("trace2");
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  registry.endTrace("trace2");
  session.stop();
  std::ifstream file("test_multiple_traces.log");
  ASSERT_TRUE(file.is_open());
  std::string line;
  int count = 0;
  while (std::getline(file, line)) {
    EXPECT_FALSE(line.empty());
    ++count;
  }
  EXPECT_EQ(count, 2);
  file.close();
  std::remove("test_multiple_traces.log");
}

TEST(yaets, MacroSharedTrace) {
  yaets::TraceSession session("test_macro_trace.log");
  SHARED_TRACE_INIT(session, "macro_trace1");
  SHARED_TRACE_INIT(session, "macro_trace2");
  SHARED_TRACE_START("macro_trace1");
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  SHARED_TRACE_END("macro_trace1");
  SHARED_TRACE_START("macro_trace2");
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  SHARED_TRACE_END("macro_trace2");
  session.stop();
  std::ifstream file("test_macro_trace.log");
  ASSERT_TRUE(file.is_open());
  std::string line;
  int count = 0;
  while (std::getline(file, line)) {
    EXPECT_FALSE(line.empty());
    ++count;
  }
  EXPECT_EQ(count, 2);
  file.close();
  std::remove("test_macro_trace.log");
}

TEST(yaets, EventsFromSeveralThreadsAreAllWritten) {
  const std::string path = "test_several_threads.log";
  {
    yaets::TraceSession session(path);
    std::vector<std::thread> producers;
    for (int t = 0; t < 4; ++t) {
      producers.emplace_back(
        [&session, t]() {
          for (int i = 0; i < 2000; ++i) {
            session.register_trace(
              "thread_" + std::to_string(t), std::chrono::nanoseconds(i),
              std::chrono::nanoseconds(i + 1));
          }
        });
    }
    for (auto & producer : producers) {
      producer.join();
    }
    session.stop();
  }
  std::ifstream file(path);
  std::string name;
  long long start = 0, end = 0;
  int count = 0;
  while (file >> name >> start >> end) {
    ++count;
  }
  EXPECT_EQ(count, 8000);
  std::remove(path.c_str());
}

TEST(yaets, StopRightAfterRegisteringWritesEverything) {
  // Many times: stopping must neither lose events nor hang.
  for (int run = 0; run < 50; ++run) {
    const std::string path = "test_stop_race.log";
    {
      yaets::TraceSession session(path);
      session.register_trace("last", std::chrono::nanoseconds(1), std::chrono::nanoseconds(2));
      session.stop();
    }
    std::ifstream file(path);
    std::string line;
    int count = 0;
    while (std::getline(file, line)) {
      ++count;
    }
    EXPECT_EQ(count, 1) << "run " << run;
    std::remove(path.c_str());
  }
}

TEST(yaets, RegisteringDoesNotWaitForASlowFile) {
  // The trace file is a FIFO nobody reads for a while: writing it blocks once the pipe is full.
  // Whoever registers traces (e.g. a real-time thread) must not block with it.
  const std::string path = "test_slow_trace.fifo";
  std::remove(path.c_str());
  ASSERT_EQ(mkfifo(path.c_str(), 0600), 0);

  std::atomic<bool> drain {false};
  int read_lines = 0;
  std::thread reader([&]() {
      std::ifstream in(path);
      while (!drain) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      std::string line;
      while (std::getline(in, line)) {
        ++read_lines;
      }
    });
  // If registering ever blocks, the reader still drains after a while, so the test ends.
  std::thread watchdog([&]() {
      const auto until = std::chrono::steady_clock::now() + std::chrono::seconds(3);
      while (!drain && std::chrono::steady_clock::now() < until) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      drain = true;
    });

  constexpr int kEvents = 20000;  // About 1 MB: far more than a pipe holds.
  {
    yaets::TraceSession session(path);
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < kEvents; ++i) {
      session.register_trace(
        "an_event_with_a_name_long_enough_to_fill_the_pipe_soon", std::chrono::nanoseconds(i),
        std::chrono::nanoseconds(i + 1));
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_FALSE(drain.load()) << "registering waited for the file";
    EXPECT_LT(elapsed, std::chrono::seconds(2));
    drain = true;
    session.stop();
  }
  reader.join();
  watchdog.join();
  EXPECT_EQ(read_lines, kEvents);
  std::remove(path.c_str());
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
