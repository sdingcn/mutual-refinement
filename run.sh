#!/bin/bash

# 1 hour, 10 GB
ulimit -t 3600 -v 10485760

# taint
for graph in exp/graphs/taint/*.dot; do
	./main "$graph" naive 0 1 > "exp/results/taint/naive-$(basename "$graph" .dot).result" 2>&1
	./main "$graph" refine 0 1 > "exp/results/taint/refine-$(basename "$graph" .dot).result" 2>&1
done

# valueflow
for graph in exp/graphs/valueflow/*.dot; do
	./main "$graph" naive 0 0 > "exp/results/valueflow/naive-$(basename "$graph" .dot).result" 2>&1
	./main "$graph" refine 0 0 > "exp/results/valueflow/refine-$(basename "$graph" .dot).result" 2>&1
done
