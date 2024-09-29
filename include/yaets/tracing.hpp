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

#ifndef YAETS__TRACING_HPP_
#define YAETS__TRACING_HPP_

#include <chrono>
#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>

namespace yaets
{

/**
 * @brief Macro to create a TraceGuard object for function tracing.
 *
 * This macro creates a TraceGuard object at the start of the function
 * to automatically trace its execution using the provided TraceSession.
 * The name of the function is automatically captured.
 */
#if defined(__GNUC__) || defined(__clang__)
    #define TRACE_EVENT(session) yaets::TraceGuard trace_guard(session, __PRETTY_FUNCTION__)
#elif defined(_MSC_VER)
    #define TRACE_EVENT(session) yaets::TraceGuard trace_guard(session, __FUNCSIG__)
#else
    #define TRACE_EVENT(session) yaets::TraceGuard trace_guard(session, __FUNCTION__)
#endif

/**
 * @brief Structure to represent a trace event.
 *
 * This structure holds the name of the function being traced, along with
 * the start and end times of the trace.
 */
struct TraceEvent
{
  std::string function_name;                  ///< Name of the traced function.
  std::chrono::nanoseconds start_time;        ///< Start time of the function in nanoseconds.
  std::chrono::nanoseconds end_time;          ///< End time of the function in nanoseconds.
};

/**
 * @brief Class to manage a trace session.
 *
 * A TraceSession manages the collection of trace events and stores them
 * in a queue. It also handles the writing of the trace events to a file
 * asynchronously via a separate consumer thread.
 */
class TraceSession
{
public:
  /**
   * @brief Construct a new TraceSession object.
   *
   * @param filename The name of the file to write the trace events to.
   */
  explicit TraceSession(const std::string & filename);

  /**
   * @brief Destroy the TraceSession object and stop tracing.
   */
  ~TraceSession();

  /**
   * @brief Stop the trace session and flush remaining trace events to the file.
   */
  void stop();

  /**
   * @brief Register a new trace event.
   *
   * This function is called by TraceGuard to register the start and end
   * time of a function execution.
   *
   * @param function_name The name of the function being traced.
   * @param start_time The start time of the function execution.
   * @param end_time The end time of the function execution.
   */
  void register_trace(
    const std::string & function_name,
    const std::chrono::nanoseconds & start_time,
    const std::chrono::nanoseconds & end_time);

private:
  std::queue<TraceEvent> trace_queue;            ///< Queue of trace events to be processed.
  std::mutex queue_mutex;                        ///< Mutex to protect access to the trace queue.
  std::condition_variable cv;                   ///< Condition variable for thread synchronization.
  std::thread consumer_thread;                   ///< Thread for processing trace events.
  std::atomic<bool> running;                     ///< Flag to indicate if the session is running.
  std::string filename_;                         ///< Name of the file to write the trace data.
  std::chrono::nanoseconds session_start_time_;  ///< Start time of the trace session.

  /**
   * @brief Consumer function that writes trace events to the file.
   *
   * This function runs in a separate thread and consumes trace events
   * from the queue, writing them to the output file.
   */
  void trace_consumer();
};

/**
 * @brief Class to manage the trace of a single function execution.
 *
 * A TraceGuard object is created at the start of a function and destroyed
 * when the function exits, automatically registering the start and end times
 * of the function execution to the associated TraceSession.
 */
class TraceGuard
{
public:
  /**
   * @brief Construct a new TraceGuard object.
   *
   * This constructor captures the function name and the start time of the function.
   *
   * @param session The TraceSession to register the trace event with.
   * @param function_name The name of the function being traced.
   */
  TraceGuard(TraceSession & session, const std::string & function_name);

  /**
   * @brief Destroy the TraceGuard object.
   *
   * The destructor automatically registers the end time of the function
   * and submits the trace event to the TraceSession.
   */
  ~TraceGuard();

private:
  TraceSession & session_;                    ///< The TraceSession associated with this trace.
  std::string function_name_;                 ///< The name of the function being traced.
  std::chrono::nanoseconds start_time_;       ///< The start time of the function execution.

protected:
  /**
   * @brief Extract the function name from the full function signature.
   *
   * This function processes the function signature to extract only the
   * relevant function name without parameters.
   *
   * @param function_signature The full function signature.
   * @return The extracted function name.
   */
  std::string extract_function_name(const std::string & function_signature);
};

}  // namespace yaets

#endif  // YAETS__TRACING_HPP_
