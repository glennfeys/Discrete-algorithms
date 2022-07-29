#!/usr/bin/env python3
"""
generate_graph.py
=================
Python script allowing the user to generate random graphs with
a specific number of edges and vertices, and a specified range of
weights.
"""
from typing import List, Tuple
import random
import argparse

def generate_edges(vertices: List[int], num_edges: int):
    edges = set()
    while len(edges) < num_edges:
        start = random.choice(vertices)
        end = random.choice(vertices)
        if start == end or (end, start) in edges:
            continue
        else:
            edges.add((start, end))
    return edges

def assign_random_weights(vertices: List[int], min_weight: int, max_weight: int):
    weights = {}
    for vertex in vertices:
        weights[vertex] = random.randint(min_weight, max_weight)
    return weights

def edge_to_string(edge: Tuple[int, int]):
    return f'e {edge[0]} {edge[1]}\n'

def weight_to_string(vertex: int, weight: int):
    return f'w {vertex} {weight}\n'

def graph_details_to_string(n: int, m: int):
    return f'p graph {n} {m}\n'

def generate_graph(out: str, n: int, m: int, min_weight: int, max_weight: int, seed=None):
    if seed:
        random.seed(seed)
    vertices = list(range(1, n+1))
    edges = generate_edges(vertices, m)
    weights = assign_random_weights(vertices, min_weight, max_weight)
    with open(out, "w+") as fp:
        fp.write(graph_details_to_string(n, m))
        for item in weights.items():
            fp.write(weight_to_string(*item))
        for edge in edges:
            fp.write(edge_to_string(edge))

parser = argparse.ArgumentParser()
parser.add_argument('--out', type=str, default="/dev/stdout", help="specify the output filename")
parser.add_argument('-n', type=int, required=True, 
    help="define the number of vertices")
parser.add_argument('-m', type=int, required=True, 
    help="define the number of edges")
parser.add_argument('--min-weight', type=int, default=1,
    help="define the minimum weight of a vertex")
parser.add_argument('--max-weight', type=int, default=20, 
    help="define the maximum weight of a vertex")
parser.add_argument('--seed', default=None, type=int, 
    help="define a seed for the random generator")

args = parser.parse_args()
generate_graph(args.out, args.n, args.m, args.min_weight, args.max_weight, args.seed)
