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
from collections import defaultdict

import matplotlib.cm as cm
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
                duration_ms = ns_to_ms(end_ns - start_ns)
                traces.append((function, start_ms, duration_ms))
    return traces


def split_labels_by_delimiter(labels, delimiter='::'):
    return [label.replace(delimiter, '\n') for label in labels]


def create_gantt_chart(traces):
    grouped_traces = defaultdict(list)
    for function, start_ms, duration_ms in traces:
        grouped_traces[function].append((start_ms, duration_ms))

    functions = list(grouped_traces.keys())
    colors = cm.get_cmap('tab10', len(functions))
    colors_by_function = {function: colors(i / len(functions)) for i,
                          function in enumerate(functions)}

    fig, ax = plt.subplots(figsize=(10, 6))

    for i, function in enumerate(functions):
        for start_ms, duration_ms in grouped_traces[function]:
            ax.barh(i, duration_ms, left=start_ms, height=0.4, color=colors_by_function[function])

    ax.set_xlabel('Time (ms)')
    ax.set_title('Gantt Chart of Traced Executions')

    split_labels = split_labels_by_delimiter(functions)
    ax.set_yticks(range(len(split_labels)))
    ax.set_yticklabels(split_labels)

    ax.grid(True, linestyle=':', linewidth=0.5)

    plt.show()


def main():
    parser = argparse.ArgumentParser(description='Generate a Gantt chart from a trace file.')
    parser.add_argument('trace_file', type=str, help='Trace file to process')
    parser.add_argument('--max_traces', type=int, default=None,
                        help='Maximum number of traces to display')

    args = parser.parse_args()

    traces = read_traces(args.trace_file, args.max_traces)

    create_gantt_chart(traces)


if __name__ == '__main__':
    main()
