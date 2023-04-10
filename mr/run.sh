#!/bin/bash

# 4 hours, 128 GB
ulimit -t 14400 -v 134217728

make -j8

# taint
for graph in exp/graphs/taint/*.dot; do
	echo "running naive on $graph"
	./main "$graph" taint naive > "exp/results/taint/naive-$(basename "$graph" .dot).result" 2>&1
	echo "running refine on $graph"
	./main "$graph" taint refine > "exp/results/taint/refine-$(basename "$graph" .dot).result" 2>&1
done

# valueflow
for graph in exp/graphs/valueflow/*.dot; do
	echo "running naive on $graph"
	./main "$graph" valueflow naive > "exp/results/valueflow/naive-$(basename "$graph" .dot).result" 2>&1
	echo "running refine on $graph"
	./main "$graph" valueflow refine > "exp/results/valueflow/refine-$(basename "$graph" .dot).result" 2>&1
done

# simplified-taint
for graph in exp/graphs/simplified-taint/*.dot; do
	echo "running refine on $graph"
	./main "$graph" taint refine > "exp/results/simplified-taint/refine-$(basename "$graph" .dot).result" 2>&1
done
