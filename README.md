
# YAETS: Yet Another Execution Tracing System

[![rolling](https://github.com/fmrico/yaets/actions/workflows/rolling.yaml/badge.svg?branch=rolling)](https://github.com/fmrico/yaets/actions/workflows/rolling.yaml)
[![jazzy-devel](https://github.com/fmrico/yaets/actions/workflows/jazzy-devel.yaml/badge.svg?branch=jazzy-devel)](https://github.com/fmrico/yaets/actions/workflows/jazzy-devel.yaml)

YAETS is a library designed to trace function execution in C++ asynchronously, combined with Python tools to analyze the results through Gantt charts and histograms.

![histogram](https://github.com/user-attachments/assets/4845320f-8736-4370-9ccb-d837426bdfaa)
![gantt](https://github.com/user-attachments/assets/754f20cd-90e6-43a9-8c18-cc523840eccb)

## Features

- Function tracing using the `TraceSession`, `TraceGuard`, `NamedSharedTrace`, and `TraceRegistry` classes.
- Asynchronous logging of trace events to prevent performance overhead.
- Python scripts to visualize traces as Gantt charts or analyze timing gaps between traces using histograms.

## Table of Contents

- [YAETS: Yet Another Execution Tracing System](#yaets-yet-another-execution-tracing-system)
  - [Features](#features)
  - [Table of Contents](#table-of-contents)
  - [Installation](#installation)
    - [Requirements](#requirements)
    - [Building the C++ Library without ROS](#building-the-c-library-without-ros)
    - [Building the C++ Library with ROS 2](#building-the-c-library-with-ros-2)
  - [Usage](#usage)
    - [C++ Tracing Library](#c-tracing-library)
      - [Example](#example)
    - [Advanced Tracing with NamedSharedTrace and TraceRegistry](#advanced-tracing-with-namedsharedtrace-and-traceregistry)
      - [NamedSharedTrace](#namedsharedtrace)
      - [Using TraceRegistry to Manage Shared Traces by ID](#using-traceregistry-to-manage-shared-traces-by-id)
    - [Using Macros with NamedSharedTrace](#using-macros-with-namedsharedtrace)
      - [Example 1](#example-1)
      - [Example 2](#example-2)
    - [How It Works](#how-it-works)
      - [Key Methods](#key-methods)
    - [Python Gantt Chart Script](#python-gantt-chart-script)
      - [Usage](#usage-1)
      - [Options](#options)
    - [Python Histogram Script for ellapsed time](#python-histogram-script-for-ellapsed-time)
      - [Usage](#usage-2)
      - [Options](#options-1)
    - [Python Histogram Script for execution time](#python-histogram-script-for-execution-time)
      - [Usage](#usage-3)
      - [Options](#options-2)
  - [Building and Running Tests](#building-and-running-tests)
  - [Tracing Session](#tracing-session)
    - [Code to trace:](#code-to-trace)
    - [Running and getting graphs](#running-and-getting-graphs)
  - [License](#license)

## Installation

### Requirements

- **C++17 or later**
- **CMake** for building the project
- **Python 3.6+** for running the scripts
- **Matplotlib** for visualizing data in Python

### Building the C++ Library without ROS

1. Clone the repository and navigate to the project directory:
    ```bash
    git clone https://github.com/fmrico/yaets.git
    cd yaets
    ```

2. Build the library using CMake:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

3. Install the library:
    ```bash
    sudo make install
    ```

### Building the C++ Library with ROS 2

1. Clone the repository and navigate to the project directory:
    ```bash
    cd my_ros_ws/src
    git clone https://github.com/fmrico/yaets.git
    cd ..
    ```

2. Build the library using Colcon:
    ```bash
    colcon build
    ```

## Usage

### C++ Tracing Library

YAETS provides a simple C++ library for tracing function execution times. The key classes are `TraceSession` and `TraceGuard`.

#### Example

```cpp
#include <yaets/tracing.hpp>

void example_function(yaets::TraceSession& session) {
    TRACE_EVENT(session);  // Automatically traces function entry and exit
    // Your function logic here
}

int main() {
    yaets::TraceSession session("trace_output.log");

    example_function(session);

    session.stop();  // Stop the session and flush all trace events
    return 0;
}
```

### Advanced Tracing with NamedSharedTrace and TraceRegistry

For scenarios where you need to trace shared events across multiple parts of your application, you can use `NamedSharedTrace` and `TraceRegistry`.

#### NamedSharedTrace

The `NamedSharedTrace` class allows you to start and stop traces from different parts of the code, under a shared trace name.

```cpp
#include <yaets/tracing.hpp>

int main() {
    yaets::TraceSession session("shared_trace_output.log");

    yaets::NamedSharedTrace trace1(session, "shared_event");
    trace1.start();
    // Perform operations
    trace1.end();

    session.stop();
    return 0;
}
```

#### Using TraceRegistry to Manage Shared Traces by ID

With `TraceRegistry`, you can register and manage multiple `NamedSharedTrace` instances by ID, allowing you to start and stop traces throughout the application without directly referencing `NamedSharedTrace` objects.

1. Initialize TraceRegistry: Register traces with IDs for centralized management.
2. Start and Stop Traces by ID: Use macros to simplify starting and stopping traces.

```cpp
#include <yaets/tracing.hpp>

int main() {
    yaets::TraceSession session("registry_trace_output.log");

    // Register traces with unique IDs
    yaets::TraceRegistry::getInstance().registerTrace("trace1", session);
    yaets::TraceRegistry::getInstance().registerTrace("trace2", session);

    // Start and end traces by ID
    yaets::TraceRegistry::getInstance().startTrace("trace1");
    // Operations for trace1
    yaets::TraceRegistry::getInstance().endTrace("trace1");

    yaets::TraceRegistry::getInstance().startTrace("trace2");
    // Operations for trace2
    yaets::TraceRegistry::getInstance().endTrace("trace2");

    session.stop();
    return 0;
}
```

### Using Macros with NamedSharedTrace

To streamline the usage of `NamedSharedTrace` with `TraceRegistry`, YAETS provides macros for initializing, starting, and stopping traces by ID. This simplifies code readability and reduces the need to call methods directly on `TraceRegistry`.

#### Example 1

```cpp
include <yaets/tracing.hpp>

int main() {
    yaets::TraceSession session("macro_trace_output.log");

    // Initialize traces by ID
    SHARED_TRACE_INIT(session, "macro_trace1");
    SHARED_TRACE_INIT(session, "macro_trace2");

    // Start and stop traces by ID
    SHARED_TRACE_START("macro_trace1");
    // Operations under macro_trace1
    SHARED_TRACE_END("macro_trace1");

    SHARED_TRACE_START("macro_trace2");
    // Operations under macro_trace2
    SHARED_TRACE_END("macro_trace2");

    session.stop();
    return 0;
}
```


#### Example 2

```cpp
class SensorDriverNode : public rclcpp::Node
{
  ...
  void produce_data()
  {
    SHARED_TRACE_START("brake_process");
    waste_time(shared_from_this(), 200us);

    sensor_msgs::msg::Image image_msg;
    pub_->publish(image_msg);
  }
...
class BrakeActuatorNode : public rclcpp::Node
{
  ...
  void react_obstacle(vision_msgs::msg::Detection3D::SharedPtr msg)
  {
    waste_time(shared_from_this(), 2ms);
    SHARED_TRACE_END("brake_process");
  } 
...
int main(int argc, char * argv[])
{
  ...
  SHARED_TRACE_INIT(session, "brake_process");
```

### How It Works

- **`TraceSession`**: Manages the tracing session and writes events asynchronously to a log file.
- **`TraceGuard`**: Automatically traces the start and end of a function. It is created at the start of the function and destroyed when the function exits, capturing the execution time.
- **`NamedSharedTrace`**: Allows you to create named traces that can be started and stopped independently across different parts of your code.
- **`TraceRegistry`**: Provides centralized management of `NamedSharedTrace` instances, allowing you to register, start, and stop traces by ID.

#### Key Methods

- `TraceSession::TraceSession(const std::string & filename)`: Initializes the session and specifies the output file for trace events.
- `TraceSession::stop()`: Stops the session and writes any remaining events to the file.
- `TraceGuard::TraceGuard(TraceSession & session, const std::string & function_name)`: Captures the function name and start time.
- `TraceGuard::~TraceGuard()`: Logs the end time and registers the trace event.
- `NamedSharedTrace::start()`: Begins a trace event.
- `NamedSharedTrace::end()`: Ends a trace event and logs it to the associated TraceSession.
- `TraceRegistry::registerTrace(const std::string& id, TraceSession& session)`: Registers a new trace with a unique ID.
- `TraceRegistry::startTrace(const std::string& id)`: Starts a trace by its ID.
- `TraceRegistry::endTrace(const std::string& id)`: Ends a trace by its ID.


### Python Gantt Chart Script

The Python script `gantt.py` allows you to visualize the traced functions as a Gantt chart.

#### Usage

1. Ensure the trace log file is generated from your C++ program (e.g., `trace_output.log`).
2. Run the `gantt.py` script:
    ```bash
    python3 scripts/gantt.py trace_output.log --max_traces 100
    ```
    or altenativelly
     ```bash
    ros2 run yaets gantt.py trace_output.log --max_traces 100
    ```

#### Options

- `--max_traces`: The maximum number of trace events to display in the Gantt chart.
  
The resulting Gantt chart shows each function's execution times, allowing you to visualize the sequence and duration of function calls.

### Python Histogram Script for ellapsed time

The Python script `elaspsed_histogram.py` analyzes the intervals between function executions and visualizes them as a histogram.

#### Usage

1. Run the `elaspsed_histogram.py` script, specifying the function name you want to analyze:
    ```bash
    python3 scripts/elaspsed_histogram.py trace_output.log --function example_function --bins 50
    ```
    or altenativelly
     ```bash
    ros2 run yaets elaspsed_histogram.py trace_output.log --function example_function --bins 50
    ```


#### Options

- `--function`: The name of the function whose execution intervals you want to analyze.
- `--bins`: The number of bins for the histogram (to control the resolution).

This tool helps you understand how frequently functions are called and whether there are patterns in the execution intervals.

### Python Histogram Script for execution time

The Python script `execution_histogram.py` analyzes execution time of a function and visualizes them as a histogram.

#### Usage

1. Run the `execution_histogram.py` script, specifying the function name you want to analyze:
    ```bash
    python3 scripts/execution_histogram.py trace_output.log --function example_function --bins 50
    ```
    or altenativelly
     ```bash
    ros2 run yaets execution_histogram.py trace_output.log --function example_function --bins 50
    ```


#### Options

- `--function`: The name of the function whose execution execution time you want to analyze.
- `--bins`: The number of bins for the histogram (to control the resolution).


## Building and Running Tests

YAETS includes unit tests to verify the functionality of the tracing library. To build and run the tests:

1. In the `build` directory, run the following commands:
    ```bash
    cd tests
    cmake ..
    make
    ./yaets_test
    ```
    or
    ```bash
    colcon test
    ```

This will run the tests in `tests/yaets_test.cpp` and verify the correctness of the tracing system.



## Tracing Session

We got the graphs at the initial of this document following these instructions:

### Code to trace:

```cpp
#include <fstream>

#include "yaets/tracing.hpp"

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;


yaets::TraceSession session("session1.log");

class ProducerNode : public rclcpp::Node
{
public:
  ProducerNode() : Node("producer_node")
  {
    pub_1_ = create_publisher<std_msgs::msg::Int32>("topic_1", 100);
    pub_2_ = create_publisher<std_msgs::msg::Int32>("topic_2", 100);
    timer_ = create_wall_timer(1ms, std::bind(&ProducerNode::timer_callback, this));
  }

  void timer_callback()
  {
    TRACE_EVENT(session);
    message_.data += 1;
    pub_1_->publish(message_);
    message_.data += 1;
    pub_2_->publish(message_);
  }

private:
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr pub_1_, pub_2_;
  rclcpp::TimerBase::SharedPtr timer_;
  std_msgs::msg::Int32 message_;
};

class ConsumerNode : public rclcpp::Node
{
public:
  ConsumerNode() : Node("consumer_node")
  {
    sub_2_ = create_subscription<std_msgs::msg::Int32>(
      "topic_2", 100, std::bind(&ConsumerNode::cb_2, this, _1));
    sub_1_ = create_subscription<std_msgs::msg::Int32>(
      "topic_1", 100, std::bind(&ConsumerNode::cb_1, this, _1));
 
    timer_ = create_wall_timer(10ms, std::bind(&ConsumerNode::timer_callback, this));
  }

  void cb_1(const std_msgs::msg::Int32::SharedPtr msg)
  {
    TRACE_EVENT(session);

    waste_time(200us);
  }

  void cb_2(const std_msgs::msg::Int32::SharedPtr msg)
  {
    TRACE_EVENT(session);

    waste_time(200us);
  }

  void timer_callback()
  {
    TRACE_EVENT(session);

    waste_time(3ms);
  }

  void waste_time(const rclcpp::Duration & duration)
  {
    auto start = now();
    while (now() - start < duration);
  }

private:
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr sub_1_;
  rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr sub_2_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node_pub = std::make_shared<ProducerNode>();
  auto node_sub = std::make_shared<ConsumerNode>();

  rclcpp::executors::MultiThreadedExecutor executor;

  executor.add_node(node_pub);
  executor.add_node(node_sub);

  executor.spin();

  rclcpp::shutdown();
  return 0;
}
```


### Running and getting graphs

Stress the system to have significant results:

```bash
stress-ng -c $(nproc) -t 300 # In terminal 1
ros2 run yaets executors     # In terminal 2
```

Stop both processes and

```bash
ros2 run yaets gantt.py ./session1.log --max_traces 200
ros2 run yaets elaspsed_histogram.py ../session1.log  --function ConsumerNode::timer_callback --bins 40
```
And we get the two graphs:

![histogram](https://github.com/user-attachments/assets/0f45055c-ea7a-46f3-9682-1de737b119c3)
![gantt](https://github.com/user-attachments/assets/56ae4f04-b294-4b84-819e-b436409c7e29)

## License

YAETS is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for more details.
