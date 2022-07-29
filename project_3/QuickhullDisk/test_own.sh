#!/bin/bash

for f in ~/The-Source-Code-and-Benchmark-Dataset-for-QuickhullDisk/test_data/**/*; do
	echo $f
	./main "$f" | grep Took
	./main "$f" | grep Took
	./main "$f" | grep Took
	./main "$f" | grep Took
	./main "$f" | grep Took
done
