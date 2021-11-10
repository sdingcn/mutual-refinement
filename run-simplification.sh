#!/bin/bash

ulimit -t 1800 -v 10485760

for dotfile in lcl-exp/taint/normal/*.dot; do
	./graph "$dotfile" > "results/simplification/$(basename "$dotfile" .dot).res" 2>&1
done
