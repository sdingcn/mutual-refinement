#!/bin/bash

# 4 hours, 128 GB
ulimit -t 14400 -v 134217728

# taint
for graph in exp/graphs/taint/*.dot; do
	echo "running naive on $graph"
	./main "$graph" naive 0 1 > "exp/results/taint/naive-$(basename "$graph" .dot).result" 2>&1
	echo "running refine on $graph"
	./main "$graph" refine 0 1 > "exp/results/taint/refine-$(basename "$graph" .dot).result" 2>&1
done

# valueflow
for graph in exp/graphs/valueflow/*.dot; do
	echo "running naive on $graph"
	./main "$graph" naive 0 0 > "exp/results/valueflow/naive-$(basename "$graph" .dot).result" 2>&1
	echo "running refine on $graph"
	./main "$graph" refine 0 0 > "exp/results/valueflow/refine-$(basename "$graph" .dot).result" 2>&1
done
