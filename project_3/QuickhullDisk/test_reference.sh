#!/bin/bash

for f in ../test_data/**/*; do
	echo $f
	./RunQuickhullDisk -i "$f" -o /dev/null | grep time
	./RunQuickhullDisk -i "$f" -o /dev/null | grep time
	./RunQuickhullDisk -i "$f" -o /dev/null | grep time
	./RunQuickhullDisk -i "$f" -o /dev/null | grep time
	./RunQuickhullDisk -i "$f" -o /dev/null | grep time
done
