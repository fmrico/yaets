#!/usr/bin/python3

# Copyright 2024 Intelligent Robotics Lab
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


import argparse

import matplotlib.pyplot as plt


def ns_to_ms(ns):
    return ns / 1_000_000


def read_traces(trace_file, function_name, max_traces=None):
    execution_times = []
    with open(trace_file, 'r') as file:
        for i, line in enumerate(file):
            if max_traces and i >= max_traces:
                break
            parts = line.split()
            if len(parts) == 3:
                function = parts[0]
                start_ns = int(parts[1])
                end_ns = int(parts[2])
                if function == function_name:
                    duration_ms = ns_to_ms(end_ns - start_ns)
                    execution_times.append(duration_ms)
    return execution_times


def create_execution_time_histogram(execution_times, function_name, num_bins):
    plt.hist(execution_times, bins=num_bins, edgecolor='black', log=True)
    plt.xlabel('Execution Time (ms)')
    plt.ylabel('Frequency (log scale)')
    plt.title(f'Execution Time Histogram for "{function_name}"')
    plt.grid(True, linestyle=':', linewidth=0.5)
    plt.show()


def main():
    parser = argparse.ArgumentParser(
        description='Generate a histogram of execution times for a specific function.')
    parser.add_argument('trace_file', type=str, help='Trace file to process')
    parser.add_argument(
        '--function', type=str, required=True, help='Name of the function for the histogram')
    parser.add_argument(
        '--max_traces', type=int, default=None, help='Maximum number of traces to display')
    parser.add_argument(
        '--bins', type=int, default=10,
        help='Number of bins (X-axis resolution) for the histogram')

    args = parser.parse_args()

    execution_times = read_traces(args.trace_file, args.function, args.max_traces)

    if not execution_times:
        print(f"No execution times found for function '{args.function}'.")
        return

    create_execution_time_histogram(execution_times, args.function, args.bins)


if __name__ == '__main__':
    main()
