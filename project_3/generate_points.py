#!/usr/bin/env python3

import random
import argparse

def generate(out: str, n: int, xmin: float, ymin: float, xmax: float, ymax: float, seed=None):
    if seed:
        random.seed(seed)

    with open(out, "w+") as file:
        for _ in range(n):
            x = random.uniform(xmin, xmax)
            y = random.uniform(ymin, ymax)
            file.write(f'{x} {y}\n')
    
parser = argparse.ArgumentParser()
parser.add_argument('--out', type=str, default="/dev/stdout", help="specify the output filename")
parser.add_argument('-n', type=int, required=True, help="define the amount of points")
parser.add_argument('--xmin', type=int, default=0, help="define the minimum value for x")
parser.add_argument('--ymin', type=int, default=0, help="define the minimum value for y")
parser.add_argument('--xmax', type=int, default=10, help="define the maximum value for x")
parser.add_argument('--ymax', type=int, default=10, help="define the maximum value for y")
parser.add_argument('--seed', default=None, type=int, help="define a seed for the random generator")

args = parser.parse_args()
generate(args.out, args.n, args.xmin, args.ymin, args.xmax, args.ymax, args.seed)
