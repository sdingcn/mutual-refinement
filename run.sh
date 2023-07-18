#!/bin/bash

ulimit -t $1 -v $2

make clean
make -j8

echo ">>> Running taint"
for dotfile in exp/taint/*.dot; do
    name=$(basename "$dotfile" .dot)
	echo "Running on $name"
	./main "exp/taint/$name.grammar" "exp/taint/$name.dot" naive \
        > "exp/taint/naive-$name.result" 2>&1
	./main "exp/taint/$name.grammar" "exp/taint/$name.dot" refine \
        > "exp/taint/refine-$name.result" 2>&1
done

echo ">>> Running valueflow"
for dotfile in exp/valueflow/*.dot; do
    name=$(basename "$dotfile" .dot)
	echo "Running on $name"
	./main "exp/valueflow/$name.grammar" "exp/valueflow/$name.dot" naive \
        > "exp/valueflow/naive-$name.result" 2>&1
	./main "exp/valueflow/$name.grammar" "exp/valueflow/$name.dot" refine \
        > "exp/valueflow/refine-$name.result" 2>&1
done

echo ">>> Running simplified-taint"
for dotfile in exp/simplified-taint/*.dot; do
    name=$(basename "$dotfile" .dot)
	echo "Running on $name"
	./main "exp/simplified-taint/$name.grammar" "exp/simplified-taint/$name.dot" refine \
        > "exp/simplified-taint/refine-$name.result" 2>&1
done
