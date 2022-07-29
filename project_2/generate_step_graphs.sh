#!/bin/bash
for (( N=10; N<=500; N*=2 )); do
for (( M=10; M<=N*(N/2); M*=2 )); do
    echo "Now generating: ${N} ${M}"
    ./generate_graph.py -n $N -m $M --min-weight 20 --max-weight 120 --seed 93 --out "Pricing/comparison/R${N}_${M}.clq"
done
done