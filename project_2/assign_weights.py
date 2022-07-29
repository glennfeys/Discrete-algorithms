#!/usr/bin/env python3
"""
assign_weights.py
=================
Python script allowing the user to assign random weights,
in a specified range, to existing `.clq` files.
"""
from typing import List, Tuple
import random
import argparse

def assign_random_weights(vertices: List[int], min_weight: int, max_weight: int):
    weights = {}
    for vertex in vertices:
        weights[vertex] = random.randint(min_weight, max_weight)
    return weights

def weight_to_string(vertex: int, weight: int):
    return f'w {vertex} {weight}\n'

def assign_weights(out: str, in_file: str, min_weight: int, max_weight: int, seed=None):
    if seed:
        random.seed(seed)
    lines = []
    n = 0
    with open(in_file, "r+") as fp:
        line = fp.readline()
        while line:
            if line.startswith('p'):
                values = line.split()
                n = int(values[2])
            if not line.startswith('w'):
                lines.append(line)
            line = fp.readline()

    vertices = list(range(1, n+1))
    weights = assign_random_weights(vertices, min_weight, max_weight)
    with open(out, "w+") as fp:
        for line in lines:
            fp.write(line)
        for item in weights.items():
            fp.write(weight_to_string(*item))

parser = argparse.ArgumentParser()
parser.add_argument('--out', type=str, default="/dev/stdout", help="specify the output filename")
parser.add_argument('input', type=str, nargs=1,
    help="Specify the input .clq file")
parser.add_argument('--min-weight', type=int, default=1,
    help="define the minimum weight of a vertex")
parser.add_argument('--max-weight', type=int, default=20, 
    help="define the maximum weight of a vertex")
parser.add_argument('--seed', default=None, type=int,
    help="define a seed for the random generator")

args = parser.parse_args()
assign_weights(args.out, args.input[0], args.min_weight, args.max_weight, args.seed)
