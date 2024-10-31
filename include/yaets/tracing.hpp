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
#include <unordered_map>
#include <memory>

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
 * @brief Macro to initialize a shared trace with a unique identifier.
 *
 * This macro registers a new NamedSharedTrace instance with the global
 * TraceRegistry, associated with the provided TraceSession.
 *
 * @param session The TraceSession to associate with the trace.
 * @param id The unique identifier for the trace.
 */
#define SHARED_TRACE_INIT(session, id) \
  yaets::TraceRegistry::getInstance().registerTrace(id, session)

/**
 * @brief Macro to start a shared trace by its unique identifier.
 *
 * This macro starts the NamedSharedTrace instance associated with the
 * provided trace identifier.
 *
 * @param id The unique identifier for the trace.
 */
#define SHARED_TRACE_START(id) \
  yaets::TraceRegistry::getInstance().startTrace(id)

/**
 * @brief Macro to end a shared trace by its unique identifier.
 *
 * This macro ends the NamedSharedTrace instance associated with the
 * provided trace identifier.
 *
 * @param id The unique identifier for the trace.
 */
#define SHARED_TRACE_END(id) \
  yaets::TraceRegistry::getInstance().endTrace(id)


/**
 * @brief Structure to represent a trace event.
 *
 * This structure holds the name of the function being traced, along with
 * the start and end times of the trace.
 */
struct TraceEvent
{
  std::string trace_name;                  ///< Name of the traced function.
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
   * @param trace_name The name of the function being traced.
   * @param start_time The start time of the function execution.
   * @param end_time The end time of the function execution.
   */
  void register_trace(
    const std::string & trace_name,
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
   * @param trace_name The name of the function being traced.
   */
  TraceGuard(TraceSession & session, const std::string & trace_name);

  /**
   * @brief Destroy the TraceGuard object.
   *
   * The destructor automatically registers the end time of the function
   * and submits the trace event to the TraceSession.
   */
  ~TraceGuard();

  /**
   * @brief Retrieve the start time of the trace.
   *
   * This getter provides read-only access to the start time recorded
   * by the `TraceGuard` at the beginning of a traced function execution.
   *
   * @return The start time of the function execution as a `std::chrono::nanoseconds` duration.
   */
  std::chrono::nanoseconds get_start_time() const
  {
    return start_time_;
  }

private:
  TraceSession & session_;                    ///< The TraceSession associated with this trace.
  std::string trace_name_;                 ///< The name of the function being traced.
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
  std::string extract_trace_name(const std::string & function_signature);
};


/**
 * @brief Class to manage shared traces across different parts of the code.
 *
 * The NamedSharedTrace class allows tracing events that can occur in
 * various parts of the code and logs both start and end times to a shared
 * TraceSession. It supports concurrent operations and capacity handling.
 */
class NamedSharedTrace
{
public:
  static const size_t TRACE_SIZE_INIT = 100; ///< Initial size for the start_times_ vector.

  /**
   * @brief Construct a new NamedSharedTrace object.
   *
   * Initializes a named trace session that can be started and ended across
   * different locations.
   *
   * @param session The TraceSession associated with the trace events.
   * @param trace_name The name of the trace event being recorded.
   */
  NamedSharedTrace(TraceSession & session, const std::string & trace_name);

  /**
   * @brief Start a new trace event.
   *
   * Records the start time of a trace event in a thread-safe manner.
   */
  void start();

  /**
   * @brief End the current trace event.
   *
   * Records the end time of a trace event and registers it with the
   * TraceSession. Ensures that a corresponding start time exists.
   */
  void end();

private:
  TraceSession & session_;                        ///< Reference to the associated TraceSession.
  std::string trace_name_;                        ///< The name of the traced event.
  std::vector<std::chrono::nanoseconds> start_times_; ///< Stores start times for trace events.

  std::atomic<size_t> counter_push_;              ///< Tracks the number of start events.
  std::atomic<size_t> counter_pop_;               ///< Tracks the number of end events.
  size_t elements_;                               ///< Current number of elements in start_times_.
  std::mutex trace_mutex_;                        ///< Mutex to ensure thread-safe operations on start_times_.
};


/**
 * @brief Singleton class to manage global NamedSharedTrace instances.
 *
 * The TraceRegistry maintains a collection of NamedSharedTrace instances,
 * identified by unique IDs, allowing traces to be started and ended globally
 * through macros and without explicit object references.
 */
class TraceRegistry
{
public:
  /**
   * @brief Retrieve the singleton instance of the TraceRegistry.
   *
   * This function returns the single, globally accessible instance of
   * the TraceRegistry.
   *
   * @return The singleton instance of TraceRegistry.
   */
  static TraceRegistry & getInstance()
  {
    static TraceRegistry instance;
    return instance;
  }

  /**
   * @brief Register a new NamedSharedTrace with a unique identifier.
   *
   * This function registers a NamedSharedTrace instance associated with
   * a given ID and TraceSession, allowing the trace to be managed globally.
   *
   * @param id The unique identifier for the trace.
   * @param session The TraceSession associated with the trace.
   */
  void registerTrace(const std::string & id, TraceSession & session);

  /**
   * @brief Start a trace identified by a unique ID.
   *
   * This function starts the NamedSharedTrace associated with the given
   * ID, marking the beginning of a traced section.
   *
   * @param id The unique identifier for the trace to start.
   */
  void startTrace(const std::string & id);

  /**
   * @brief End a trace identified by a unique ID.
   *
   * This function ends the NamedSharedTrace associated with the given
   * ID, marking the end of a traced section.
   *
   * @param id The unique identifier for the trace to end.
   */
  void endTrace(const std::string & id);

private:
  std::unordered_map<std::string, std::unique_ptr<NamedSharedTrace>> traces_; ///< Stores traces by unique ID.
  std::mutex mutex_; ///< Mutex for synchronizing access to the trace map.

  // Private constructors for singleton pattern
  TraceRegistry() = default;
  ~TraceRegistry() = default;

  // Disable copy construction and assignment
  TraceRegistry(const TraceRegistry &) = delete;
  TraceRegistry & operator=(const TraceRegistry &) = delete;
};

}  // namespace yaets

#endif  // YAETS__TRACING_HPP_
