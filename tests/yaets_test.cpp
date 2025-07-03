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
#include <fstream>
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

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
