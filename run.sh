#!/bin/bash

echo "STEP 1 >>> running valueflow analysis to get the graphs"
cd vf/
./analyze.sh
cd ../

echo "STEP 2 >>> running graph simplification to get the graphs"
touch lzr_time.txt
for graph in mr/exp/graphs/taint/*.dot; do
	echo "running graph simplification on $graph"
	cp $graph lzr/current.dot
	cd lzr/
	./graph_reduce.sh > output
	echo $(basename "$graph" .dot) >> ../lzr_time.txt
	awk 'END {print $NF}' output >> ../lzr_time.txt
	cd ../
done
rm lzr/output

echo "STEP 3 >>> running mutual refinement"
cd mr/
./run.sh
cd ../

echo "STEP 4 >>> reporting results"
python3 report.py
