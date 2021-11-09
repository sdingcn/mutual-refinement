#!/bin/bash

ulimit -t 1800 -v 31457280

for dotfile in lcl-exp/taint/normal/*.dot; do
	./naive "$dotfile" > "results/reachability/naive-$(basename "$dotfile" .dot).res" 2>&1
	./refine "$dotfile" > "results/reachability/refine-$(basename "$dotfile" .dot).res" 2>&1
done
