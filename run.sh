#!/bin/bash

# 4 hours, 128 GB
ulimit -t 14400 -v 134217728

make

# taint
for graph in exp/graphs/taint/*.dot; do
	echo "running naive on $graph"
	./main "$graph" naive taint > "exp/results/taint/naive-$(basename "$graph" .dot).result" 2>&1
	echo "running refine on $graph"
	./main "$graph" refine taint > "exp/results/taint/refine-$(basename "$graph" .dot).result" 2>&1
done

# valueflow
for graph in exp/graphs/valueflow/*.dot; do
	echo "running naive on $graph"
	./main "$graph" naive valueflow > "exp/results/valueflow/naive-$(basename "$graph" .dot).result" 2>&1
	echo "running refine on $graph"
	./main "$graph" refine valueflow > "exp/results/valueflow/refine-$(basename "$graph" .dot).result" 2>&1
done
