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
#ifndef _WIN32
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <algorithm>
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

#ifndef _WIN32  // FIFOs are POSIX.
TEST(yaets, RegisteringDoesNotWaitForASlowFile) {
  // The trace file is a FIFO nobody reads for a while: once the pipe is full, the consumer is
  // blocked writing it. Whoever registers traces (e.g. a real-time thread) must not block with it.
  const std::string path = "test_slow_trace.fifo";
  std::remove(path.c_str());
  ASSERT_EQ(mkfifo(path.c_str(), 0600), 0);

  // Opened before the session, so the consumer can open it for writing.
  const int reader = open(path.c_str(), O_RDONLY | O_NONBLOCK);
  ASSERT_GE(reader, 0);
#ifdef F_GETPIPE_SZ
  const int capacity = fcntl(reader, F_GETPIPE_SZ);
#else
  const int capacity = 65536;
#endif
  ASSERT_GT(capacity, 0);

  auto pending_in_pipe = [reader]() {
      int bytes = 0;
      ioctl(reader, FIONREAD, &bytes);
      return bytes;
    };

  std::atomic<bool> drain {false};
  int read_lines = 0;
  std::thread drainer([&]() {
      while (!drain) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      char buffer[4096];
      while (true) {
        const ssize_t n = read(reader, buffer, sizeof(buffer));
        if (n > 0) {
          read_lines += static_cast<int>(std::count(buffer, buffer + n, '\n'));
        } else if (n == 0) {
          break;  // The consumer closed the file: all written.
        } else {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
      }
    });
  // If registering ever blocks, the pipe is drained after a while anyway, so the test ends.
  std::thread watchdog([&]() {
      const auto until = std::chrono::steady_clock::now() + std::chrono::seconds(3);
      while (!drain && std::chrono::steady_clock::now() < until) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      drain = true;
    });

  const std::string name = "an_event_with_a_name_long_enough_to_fill_the_pipe_soon";
  int registered = 0;
  {
    yaets::TraceSession session(path);

    // 1. Fill the pipe, until the consumer is certainly blocked writing it: far more registered
    // than the pipe holds, and what it holds no longer changes. (A full pipe may hold a bit less
    // than its nominal capacity: Linux fills it by pages.)
    const auto line_bytes = static_cast<int>(name.size()) + 5;  // "<name> 1 2\n"
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    int last_pending = -1;
    auto stable_since = std::chrono::steady_clock::now();
    bool consumer_blocked = false;
    while (!consumer_blocked && !drain && std::chrono::steady_clock::now() < deadline) {
      for (int i = 0; i < 100; ++i, ++registered) {
        session.register_trace(name, std::chrono::nanoseconds(1), std::chrono::nanoseconds(2));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      const int pending = pending_in_pipe();
      if (pending != last_pending) {
        last_pending = pending;
        stable_since = std::chrono::steady_clock::now();
      }
      consumer_blocked = pending > 0 && registered * line_bytes > 2 * capacity &&
        std::chrono::steady_clock::now() - stable_since > std::chrono::milliseconds(100);
    }
    // Not ASSERT: the helper threads must always be joined.
    consumer_blocked = consumer_blocked && !drain;
    EXPECT_TRUE(consumer_blocked) << "registering waited for the file, or the pipe never filled";

    // 2. With the consumer blocked, registering must not wait for it.
    if (consumer_blocked) {
      const auto start = std::chrono::steady_clock::now();
      for (int i = 0; i < 1000; ++i, ++registered) {
        session.register_trace(name, std::chrono::nanoseconds(1), std::chrono::nanoseconds(2));
      }
      const auto elapsed = std::chrono::steady_clock::now() - start;
      EXPECT_FALSE(drain.load()) << "registering waited for the file";
      EXPECT_LT(elapsed, std::chrono::seconds(1));
    }

    drain = true;
    session.stop();
  }
  drainer.join();
  watchdog.join();
  close(reader);
  EXPECT_EQ(read_lines, registered);
  std::remove(path.c_str());
}
#endif

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
