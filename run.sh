#!/bin/bash

echo "STEP 1 >>> running valueflow analysis to get the graphs"
cd vf/
./analyze.sh
cd ../

echo "STEP 2 >>> running graph simplification to get the graphs"
for graph in mr/exp/graphs/taint/*.dot; do
	echo "running graph simplification on $graph"
	cp $graph lzr/current.dot
	cd lzr/
	./graph_reduce.sh > resource-logs/$(basename "$graph" .dot).log 2>&1
	cp current.dot ../mr/exp/graphs/simplified-taint/$(basename "$graph")
	cd ../
done

echo "STEP 3 >>> running mutual refinement"
cd mr/
./run.sh
cd ../
