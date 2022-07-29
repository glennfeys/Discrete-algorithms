#!/bin/bash
for f in TestData/DimacsAdapted_1_20/*.clq; do
    echo $f
    (gtime gtimeout 600 './Branch and bound/main' $f) 2>&1
done
