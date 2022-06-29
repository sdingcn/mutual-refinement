#!/bin/bash

# 1 hour, 10 GB
ulimit -t 3600 -v 10485760

for graph in resource/graphs/*.dot; do
	./naive "$graph" > "resource/results/naive-$(basename "$graph" .dot).result" 2>&1
done

for graph in resource/graphs/*.dot; do
	./refine "$graph" > "resource/results/refine-$(basename "$graph" .dot).result" 2>&1
done
