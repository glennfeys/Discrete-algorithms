#!/bin/bash
BENCHMARKS=comparison/*.clq
echo "n,m,min_degree,max_degree,total_degree,min_weight,max_weight,none,"
for f in $BENCHMARKS
do
echo -n "${f##*/}," | sed 's/R//' | sed 's/.clq//' | sed 's/_/,/'
for method in min_degree max_degree total_degree min_weight max_weight none
do
  # take action on each file. $f store current file name
  result=$(./main $f $method)

  value=$(echo "$result"  | grep 'Minimum weight of vertex cover:' | sed 's/Minimum weight of vertex cover: //')


  echo -n "$value,";
done
echo ""
done