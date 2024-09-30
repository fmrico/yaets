
# YAETS: Yet Another Execution Tracing System

[![rolling](https://github.com/fmrico/yaest/actions/workflows/rolling.yaml/badge.svg?branch=rolling)](https://github.com/fmrico/yaest/actions/workflows/rolling.yaml)

YAETS is a library designed to trace function execution in C++ asynchronously, combined with Python tools to analyze the results through Gantt charts and histograms.

![histogram](https://github.com/user-attachments/assets/4845320f-8736-4370-9ccb-d837426bdfaa)
![gantt](https://github.com/user-attachments/assets/754f20cd-90e6-43a9-8c18-cc523840eccb)

## Features

- Function tracing using the `TraceSession` and `TraceGuard` classes.
- Asynchronous logging of trace events to prevent performance overhead.
- Python scripts to visualize traces as Gantt charts or analyze timing gaps between traces using histograms.

## Table of Contents

- [YAETS: Yet Another Execution Tracing System](#yaets-yet-another-execution-tracing-system)
  - [Features](#features)
  - [Table of Contents](#table-of-contents)
  - [Installation](#installation)
    - [Requirements](#requirements)
    - [Building the C++ Library](#building-the-c-library)
  - [Usage](#usage)
    - [C++ Tracing Library](#c-tracing-library)
      - [Example](#example)
    - [How It Works](#how-it-works)
      - [Key Methods](#key-methods)
    - [Python Gantt Chart Script](#python-gantt-chart-script)
      - [Usage](#usage-1)
      - [Options](#options)
    - [Python Histogram Script](#python-histogram-script)
      - [Usage](#usage-2)
      - [Options](#options-1)
  - [Building and Running Tests](#building-and-running-tests)
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

### How It Works

- **`TraceSession`**: Manages the tracing session and writes events asynchronously to a log file.
- **`TraceGuard`**: Automatically traces the start and end of a function. It is created at the start of the function and destroyed when the function exits, capturing the execution time.

#### Key Methods

- `TraceSession::TraceSession(const std::string & filename)`: Initializes the session and specifies the output file for trace events.
- `TraceSession::stop()`: Stops the session and writes any remaining events to the file.
- `TraceGuard::TraceGuard(TraceSession & session, const std::string & function_name)`: Captures the function name and start time.
- `TraceGuard::~TraceGuard()`: Logs the end time and registers the trace event.

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
    ros2 run yaest gantt.py trace_output.log --max_traces 100
    ```

#### Options

- `--max_traces`: The maximum number of trace events to display in the Gantt chart.
  
The resulting Gantt chart shows each function's execution times, allowing you to visualize the sequence and duration of function calls.

### Python Histogram Script

The Python script `histogram.py` analyzes the intervals between function executions and visualizes them as a histogram.

#### Usage

1. Run the `histogram.py` script, specifying the function name you want to analyze:
    ```bash
    python3 scripts/histogram.py trace_output.log --function example_function --bins 50
    ```
    or altenativelly
     ```bash
    ros2 run yaest histogram.py trace_output.log --function example_function --bins 50
    ```


#### Options

- `--function`: The name of the function whose execution intervals you want to analyze.
- `--bins`: The number of bins for the histogram (to control the resolution).

This tool helps you understand how frequently functions are called and whether there are patterns in the execution intervals.

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
ros2 run yaest gantt.py ./session1.log --max_trazas 200
ros2 run yaest histogram.py ../session1.log  --funcion ConsumerNode::timer_callback --bins 40
```
And we get the two graphs:

![histogram](https://github.com/user-attachments/assets/0f45055c-ea7a-46f3-9682-1de737b119c3)
![gantt](https://github.com/user-attachments/assets/56ae4f04-b294-4b84-819e-b436409c7e29)

## License

YAETS is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for more details.
