#!/usr/bin/python

# Copyright 2025 Intelligent Robotics Lab
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
from collections import defaultdict
import statistics


def ns_to_ms(ns):
    return ns / 1_000_000


def read_traces(trace_file):
    traces = []
    with open(trace_file, 'r') as file:
        for line in file:
            parts = line.split()
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
    for function, start_ms, end_ms in traces:
        grouped[function].append((start_ms, end_ms))
    return grouped


def print_stats(grouped_traces):
    for function, executions in grouped_traces.items():
        durations = [end - start for start, end in executions]
        starts = sorted([start for start, _ in executions])
        intervals = [starts[i+1] - starts[i] for i in range(len(starts) - 1)]

        print(f'\nFunction: {function}')
        print(f'  Executions: {len(durations)}')

        if durations:
            print(f'  Duration [ms]      -> Mean: {statistics.mean(durations):.3f}, '
                  f'Std Dev: {statistics.stdev(durations) if len(durations) > 1 else 0.0:.3f}')
        else:
            print('  Duration [ms]      -> No data')

        if intervals:
            print(f'  Interval [ms]      -> Mean: {statistics.mean(intervals):.3f}, '
                  f'Std Dev: {statistics.stdev(intervals) if len(intervals) > 1 else 0.0:.3f}')
        else:
            print('  Interval [ms]      -> Not enough data')


def main():
    parser = argparse.ArgumentParser(
        description='Compute execution time statistics from a trace file.')
    parser.add_argument('trace_file', type=str, help='Trace file to process')

    args = parser.parse_args()

    traces = read_traces(args.trace_file)
    grouped = group_traces_by_function(traces)

    print_stats(grouped)


if __name__ == '__main__':
    main()
