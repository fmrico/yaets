
# YAETS: Yet Another Execution Tracing System

YAETS is a library designed to trace function execution in C++ asynchronously, combined with Python tools to analyze the results through Gantt charts and histograms.

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

### Building the C++ Library

1. Clone the repository and navigate to the project directory:
    ```bash
    git clone https://github.com/your-repo/yaets.git
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

This will run the tests in `tests/yaets_test.cpp` and verify the correctness of the tracing system.

## License

YAETS is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for more details.
