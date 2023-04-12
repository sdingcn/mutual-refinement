#!/bin/bash

echo "STEP 1 >>> running valueflow analysis to get the graphs"
cd vf/
./analyze.sh
cd ../

echo "STEP 2 >>> running graph simplification to get the graphs"
touch lzr-time.txt
for graph in mr/exp/graphs/taint/*.dot; do
	echo "running graph simplification on $graph"
	cp $graph lzr/current.dot
	cd lzr/
	./graph_reduce.sh > output
	echo $(basename "$graph" .dot) >> ../lzr-time.txt
	awk 'END {print $NF}' output >> ../lzr-time.txt
	cp current.dot ../mr/exp/graphs/simplified-taint/$(basename "$graph")
	cd ../
done
rm lzr/output

echo "STEP 3 >>> running mutual refinement"
cd mr/
./run.sh
cd ../
