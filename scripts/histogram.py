#!/usr/bin/python

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


def read_traces(trace_file, max_traces=None):
    traces = []
    with open(trace_file, 'r') as file:
        for i, line in enumerate(file):
            if max_traces and i >= max_traces:
                break
            parts = line.split()
            if len(parts) == 3:
                function = parts[0]
                start_ns = int(parts[1])
                end_ns = int(parts[2])
                start_ms = ns_to_ms(start_ns)
                end_ms = ns_to_ms(end_ns)
                traces.append((function, start_ms, end_ms))
    return traces


def filter_traces_by_function(traces, target_function):
    return [(start_ms, end_ms) for
            function, start_ms, end_ms in traces if function == target_function]


def calculate_intervals_between_starts(traces):
    ordered_traces = sorted(traces, key=lambda x: x[0])
    intervals = [ordered_traces[i+1][0] - ordered_traces[i][0] for
                 i in range(len(ordered_traces) - 1)]
    return intervals


def create_histogram(intervals, target_function, num_bins):
    plt.hist(intervals, bins=num_bins, edgecolor='black', log=True)
    plt.xlabel('Interval between starts (ms)')
    plt.ylabel('Frequency (log scale)')
    plt.title(f'Histogram of intervals between starts of "{target_function}"')
    plt.show()


def main():
    parser = argparse.ArgumentParser(
        description='Generate a Gantt chart or histogram from a trace file.')
    parser.add_argument('trace_file', type=str, help='Trace file to process')
    parser.add_argument('--function', type=str, required=True,
                        help='Name of the function for the histogram')
    parser.add_argument('--max_traces', type=int, default=None,
                        help='Maximum number of traces to display')
    parser.add_argument('--bins', type=int, default=10,
                        help='Number of bins (X-axis resolution) for the histogram')

    args = parser.parse_args()

    traces = read_traces(args.trace_file, args.max_traces)

    function_traces = filter_traces_by_function(traces, args.function)

    if len(function_traces) < 2:
        print(f"There are not enough executions of the function '{args.function}' "
              'to calculate intervals.')
        return

    intervals = calculate_intervals_between_starts(function_traces)

    create_histogram(intervals, args.function, args.bins)


if __name__ == '__main__':
    main()
