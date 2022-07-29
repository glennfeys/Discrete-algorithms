#!/bin/bash
for f in ../../tests/*.txt; do
    echo $f
    (time timeout 600 './GrahamScan/main' $f) 2>&1
done
