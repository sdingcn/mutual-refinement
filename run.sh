#!/bin/bash

ulimit -t $1 -v $2

make clean

make -j8

# taint
for graph in exp/taint/*.dot; do
	echo "running naive on $graph"
	./main "$graph" taint naive > "exp/taint/naive-$(basename "$graph" .dot).result" 2>&1
	echo "running refine on $graph"
	./main "$graph" taint refine > "exp/taint/refine-$(basename "$graph" .dot).result" 2>&1
done

# valueflow
for graph in exp/valueflow/*.dot; do
	echo "running naive on $graph"
	./main "$graph" valueflow naive > "exp/valueflow/naive-$(basename "$graph" .dot).result" 2>&1
	echo "running refine on $graph"
	./main "$graph" valueflow refine > "exp/valueflow/refine-$(basename "$graph" .dot).result" 2>&1
done

# simplified-taint
for graph in exp/simplified-taint/*.dot; do
	echo "running refine on $graph"
	./main "$graph" taint refine > "exp/simplified-taint/refine-$(basename "$graph" .dot).result" 2>&1
done
