#!/bin/bash

# 1 hour, 10 GB
ulimit -t 3600 -v 10485760

for graph in exp/graphs/*.dot; do
	./naive "$graph" > "exp/results/naive-$(basename "$graph" .dot).result" 2>&1
	./refine "$graph" > "exp/results/refine-$(basename "$graph" .dot).result" 2>&1
done
