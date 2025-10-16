#!/usr/bin/python3

# Copyright 2025 Intelligent Robotics Lab
#
# Licensed under the Apache License, Version 2.0 (the "License");
# You may not use this file except in compliance with the License.
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
from collections import defaultdict
import os

import matplotlib.pyplot as plt


def ns_to_ms(ns):
    return ns / 1_000_000


def read_traces(trace_file):
    traces = []
    with open(trace_file, 'r') as file:
        for line in file:
            parts = line.strip().split()
            if len(parts) == 3:
                function = parts[0]
                start_ns = int(parts[1])
                end_ns = int(parts[2])
                start_ms = ns_to_ms(start_ns)
                end_ms = ns_to_ms(end_ns)
                traces.append((function, start_ms, end_ms))
    return traces


def group_traces_by_function(traces):
    grouped = defaultdict(list)
    for function, start, end in traces:
        grouped[function].append((start, end))
    return grouped


def save_execution_time_histogram(execution_times, function_name, output_dir, font_size):
    filename = f'execution_{function_name.replace("::", "_")}.pdf'
    filepath = os.path.join(output_dir, filename)

    plt.figure()
    plt.rcParams.update({'font.size': font_size})
    plt.hist(execution_times, bins=20, edgecolor='black', log=True)
    plt.xlabel('Execution Time (ms)')
    plt.ylabel('Frequency (log scale)')
    plt.title(f'Execution Time Histogram\n{function_name}')
    plt.grid(True, linestyle=':', linewidth=0.5)
    plt.tight_layout()
    plt.savefig(filepath)
    plt.close()


def save_elapsed_time_histogram(start_times, function_name, output_dir, font_size):
    intervals = [start_times[i+1] - start_times[i] for i in range(len(start_times) - 1)]
    if not intervals:
        return

    filename = f'elapsed_{function_name.replace("::", "_")}.pdf'
    filepath = os.path.join(output_dir, filename)

    plt.figure()
    plt.rcParams.update({'font.size': font_size})
    plt.hist(intervals, bins=20, edgecolor='black', log=True)
    plt.xlabel('Interval between starts (ms)')
    plt.ylabel('Frequency (log scale)')
    plt.title(f'Elapsed Time Histogram\n{function_name}')
    plt.grid(True, linestyle=':', linewidth=0.5)
    plt.tight_layout()
    plt.savefig(filepath)
    plt.close()


def main():
    parser = argparse.ArgumentParser(
        description='Generate histograms for execution and interval times per function.')
    parser.add_argument('trace_file', type=str, help='Trace file to process')
    parser.add_argument('--font_size', type=int, default=10,
                        help='Base font size for plot labels and titles')

    args = parser.parse_args()

    traces = read_traces(args.trace_file)
    grouped_traces = group_traces_by_function(traces)
    output_dir = os.path.dirname(os.path.abspath(args.trace_file))

    for function, executions in grouped_traces.items():
        start_times = sorted([start for start, _ in executions])
        durations = [end - start for start, end in executions]

        if durations:
            save_execution_time_histogram(durations, function, output_dir, args.font_size)
        if len(start_times) > 1:
            save_elapsed_time_histogram(start_times, function, output_dir, args.font_size)


if __name__ == '__main__':
    main()
