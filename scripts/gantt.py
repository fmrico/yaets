#!/usr/bin/python

import matplotlib.pyplot as plt
import argparse
import re
import matplotlib.cm as cm

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

def create_gantt_chart(traces):
    functions = list(set(function for function, _, _ in traces))
    colors = cm.get_cmap('tab10', len(functions))
    colors_by_function = {function: colors(i) for i, function in enumerate(functions)}

    fig, ax = plt.subplots(figsize=(10, 6))

    for i, (function, start_ms, duration_ms) in enumerate(traces):
        ax.barh(function, duration_ms, left=start_ms, height=0.4, color=colors_by_function[function])

    ax.set_xlabel('Time (ms)')
    ax.set_ylabel('Functions')
    ax.set_title('Gantt Chart of Traced Executions')
    ax.grid(True, linestyle=':', linewidth=0.5)

    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Generate a Gantt chart from a trace file.')
    parser.add_argument('trace_file', type=str, help='Trace file to process')
    parser.add_argument('--max_traces', type=int, default=None, help='Maximum number of traces to display')

    args = parser.parse_args()

    traces = read_traces(args.trace_file, args.max_traces)

    create_gantt_chart(traces)

if __name__ == '__main__':
    main()
