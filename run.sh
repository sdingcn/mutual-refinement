#!/bin/bash

echo "STEP 1 >>> running valueflow analysis to get the graphs"
cd vf/
./analyze.sh
cd ../

echo "STEP 2 >>> running graph simplification to get the graphs"
for graph in mr/exp/graphs/taint/*.dot; do
	echo "running graph simplification on $graph"
	cp graph lzr/current.dot
	cd lzr/
	./graph_reduce.sh > output
	$(basename "$graph" .dot) >> ../lzr_time.txt
	awk 'END {print $NF}' output >> ../lzr_time.txt
	echo "\n" >> ../lzr_time.txt
	cp current.dot ../mr/exp/graphs/simplified-taint/$(basename "$graph" .dot).dot
	cd ../
done

echo "STEP 3 >>> running mutual refinement"
cd mr/
./run.sh
cd ../

echo "STEP 4 >>> reporting results"
python3 report.py
