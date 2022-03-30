#!/bin/bash

ulimit -t 3600 -v 10485760

for graph in sample-graphs/*.dot; do
	./naive "$graph" > "results/naive-$(basename "$graph" .dot).res" 2>&1
done

for graph in sample-graphs/*.dot; do
	./refine "$graph" > "results/refine-$(basename "$graph" .dot).res" 2>&1
done
