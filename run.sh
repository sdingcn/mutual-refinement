#!/bin/bash

ulimit -t 1800 -v 10485760

for graph in sample-graphs/*.dot; do
	./main "$graph" > "results/main-$(basename "$graph" .dot).res" 2>&1
done
