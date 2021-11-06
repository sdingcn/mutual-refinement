# mutual-refinement-of-graph-reachability

## dependencies
This implementation is intended to be used on Linux.
You need `make` and `g++` (supporting at least C++11).

## compilation
There are 3 targets: naive, refine, graph.
They represent the naive combination, the mutual-refinement-based combination, and the mutual-refinement-based graph simplification, respectively.
`make` compiles all of them.
You can also compile them individually via `make naive`, `make refine`, and `make graph`.
`make clean` removes all compiled files.

## usage
Running `./naive`, `./refine`, or `./graph` prints the usage.
