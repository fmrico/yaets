// Copyright 2024 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "yaets/tracing.hpp"

#include "gtest/gtest.h"


class TraceGuardTest : public yaets::TraceGuard
{
public:
  TraceGuardTest(yaets::TraceSession & session, const std::string & function_name)
  : TraceGuard(session, function_name)
  {
  }

  std::string extract_function_name_test(const std::string & function_signature)
  {
    return extract_function_name(function_signature);
  }
};


TEST(yaets, extract_function_name)
{
  yaets::TraceSession trace_session("no_file.log");
  TraceGuardTest trace_guard(trace_session, "no_file");

  ASSERT_EQ(trace_guard.extract_function_name_test("function_1(std::string args)"), "function_1");
  ASSERT_EQ(trace_guard.extract_function_name_test("function_1()"), "function_1");
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
