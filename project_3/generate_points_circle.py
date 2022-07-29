#!/usr/bin/env python3
# Generate random benchmark data for the default problem
########################################################
#
# This command utilises the equation of a circle to choose the outer points of the convex hull at random.
# Then it fills this outer hull to a specified density. This way we can precisely compare specific sizes of
# convex hulls among the algorithms.

import random
import argparse
import math
from shapely.geometry import Point
from shapely.geometry.polygon import Polygon

def generate(out: str, n: int, d: float, R: float, seed=None):
    if seed:
        random.seed(seed)

    M = random.randint(-100, 100), random.randint(-100, 100)

    with open(out, "w+") as file:
        # Step 1: Generate the points on our hull.
        circumference_points = []
        for _ in range(n):
            a = random.random() * 2 * math.pi
            r = R

            x = r * math.cos(a) + M[0]
            y = r * math.sin(a) + M[1]
            circumference_points.append((x, y))

        circumference = Polygon(circumference_points)

        # Step 2: Generate some inner points while taking into
        #         consideration if these points are inside of our hull

        inner_points = []
        while len(inner_points) < (int(d * math.pi * R**2)):
            a = random.random() * 2 * math.pi
            r = R * math.sqrt(random.random())
            
            x = r * math.cos(a) + M[0]
            y = r * math.sin(a) + M[1]
            if circumference.contains(Point(x,y)):
                inner_points.append((x, y))

        for (x,y) in circumference_points + inner_points:
            file.write(f'{x:.6f} {y:.6f}\n')

    
parser = argparse.ArgumentParser()
parser.add_argument('--out', type=str, default="/dev/stdout", help="specify the output filename")
parser.add_argument('-n', type=int, required=True, help="define the minimum amount of points on the hull, e.g. 30")
parser.add_argument('-d', type=float, default=1.0, help="defines the amount of points per unit of surface of the surrounding circle")
parser.add_argument('-r', type=float, default=10.0, help="defines the radius of the circle surrounding the hull")
parser.add_argument('--seed', default=None, type=int, help="define a seed for the random generator")

args = parser.parse_args()
generate(args.out, args.n, args.d, args.r, args.seed)
