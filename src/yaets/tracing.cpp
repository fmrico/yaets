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


#include <chrono>
#include <string>
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <fstream>

#include "yaets/tracing.hpp"

namespace yaets
{

TraceSession::TraceSession(const std::string & filename)
: running(true), filename_(filename),
  session_start_time_(std::chrono::high_resolution_clock::now().time_since_epoch())
{
  consumer_thread = std::thread(&TraceSession::trace_consumer, this);
}

TraceSession::~TraceSession()
{
  stop();
}

void
TraceSession::stop()
{
  if (running) {
    running = false;
    cv.notify_all();
    if (consumer_thread.joinable()) {
      consumer_thread.join();
    }
  }
}

void
TraceSession::register_trace(
  const std::string & function_name,
  const std::chrono::nanoseconds & start_time,
  const std::chrono::nanoseconds & end_time)
{
  TraceEvent event;
  event.function_name = function_name;
  event.start_time = start_time - session_start_time_;
  event.end_time = end_time - session_start_time_;

  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    trace_queue.push(event);
  }

  cv.notify_one();
}

void
TraceSession::trace_consumer()
{
  std::ofstream trace_file(filename_);

  while (running || !trace_queue.empty()) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv.wait(lock, [this] {return !trace_queue.empty() || !running;});

    while (!trace_queue.empty()) {
      TraceEvent event = trace_queue.front();
      trace_queue.pop();

      trace_file << event.function_name
                 << " " << event.start_time.count()
                 << " " << event.end_time.count() << "\n";
    }
  }

  trace_file.close();
}

TraceGuard::TraceGuard(TraceSession & session, const std::string & function_name)
: session_(session), function_name_(extract_function_name(function_name)),
  start_time_(std::chrono::high_resolution_clock::now().time_since_epoch())
{
}

TraceGuard::~TraceGuard()
{
  auto end_time = std::chrono::high_resolution_clock::now().time_since_epoch();
  session_.register_trace(function_name_, start_time_, end_time);
}


std::string
TraceGuard::extract_function_name(const std::string & function_signature)
{
  std::string result = function_signature;

  size_t pos = result.find('(');
  if (pos != std::string::npos) {
    result = result.substr(0, pos);
  }

  pos = result.rfind(' ');
  if (pos != std::string::npos) {
    result = result.substr(pos + 1);
  }

  return result;
}

}  // namespace yaets
