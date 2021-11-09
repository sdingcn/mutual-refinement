#!/bin/bash

ulimit -t 1000 -v 10485760

for dotfile in lcl-exp/taint/normal/*.dot; do
	./naive "$dotfile" > "results/reachability/naive-$(basename "$dotfile" .dot).res"
	./refine "$dotfile" > "results/reachability/refine-$(basename "$dotfile" .dot).res"
done
