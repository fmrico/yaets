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
  const std::string & trace_name,
  const std::chrono::nanoseconds & start_time,
  const std::chrono::nanoseconds & end_time)
{
  TraceEvent event;
  event.trace_name = trace_name;
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

      trace_file << event.trace_name
                 << " " << event.start_time.count()
                 << " " << event.end_time.count() << "\n";
      trace_file.flush();
    }
  }

  trace_file.close();
}

TraceGuard::TraceGuard(TraceSession & session, const std::string & trace_name)
: session_(session), trace_name_(extract_trace_name(trace_name)),
  start_time_(std::chrono::high_resolution_clock::now().time_since_epoch())
{
}

TraceGuard::~TraceGuard()
{
  auto end_time = std::chrono::high_resolution_clock::now().time_since_epoch();
  session_.register_trace(trace_name_, start_time_, end_time);
}


std::string
TraceGuard::extract_trace_name(const std::string & function_signature)
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

NamedSharedTrace::NamedSharedTrace(TraceSession & session, const std::string & trace_name)
: session_(session), trace_name_(trace_name),
  counter_push_(0), counter_pop_(0), elements_(0), start_times_(TRACE_SIZE_INIT)
{
}

void
NamedSharedTrace::start()
{
  std::lock_guard<std::mutex> lock(trace_mutex_);

  if (elements_ >= start_times_.size()) {
    std::cerr << "Warning: Start times vector is full. Cannot start new trace." << std::endl;
    return;
  }
  elements_++;

  start_times_[counter_push_] = std::chrono::high_resolution_clock::now().time_since_epoch();
  counter_push_ = (counter_push_ + 1) % start_times_.size();
}

void
NamedSharedTrace::end()
{
  std::lock_guard<std::mutex> lock(trace_mutex_);

  if (elements_ == 0) {
    std::cerr << "Warning: No matching start() call for end() - ignoring." << std::endl;
    return;
  }

  auto end_time = std::chrono::high_resolution_clock::now().time_since_epoch();
  session_.register_trace(trace_name_, start_times_[counter_pop_], end_time);
  counter_pop_ = (counter_pop_ + 1) % start_times_.size();
  elements_--;
}

void
TraceRegistry::registerTrace(const std::string & id, TraceSession & session)
{
  std::lock_guard<std::mutex> lock(mutex_);
  traces_[id] = std::make_unique<NamedSharedTrace>(session, id);
}

void
TraceRegistry::startTrace(const std::string & id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (traces_.count(id)) {
    traces_[id]->start();
  }
}

void
TraceRegistry::endTrace(const std::string & id)
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (traces_.count(id)) {
    traces_[id]->end();
  }
}

}  // namespace yaets
