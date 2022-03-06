# mutual-refinement-of-CFL-reachability

## dependencies
This implementation is intended to be used on Linux.
You need `make` and `g++` (supporting at least C++11).

## compilation
`make`

## clean
`make clean`

## usage
`./main <graph-file-path> (<grammar-file-path>)+`

(Running `./main` prints the usage.)

## input specification

### Grammar File
```
<nonterminal>	[A-Z][-_A-Za-z0-9]*
<terminal>	[a-z][-_A-Za-z0-9]*
<symbol>	(<nonterminal> | <terminal>)
<space>		[ \t]+
<newline>	\n
<line>		(<nonterminal> | <nonterminal> <space> <symbol> | <nonterminal> <space> <symbol> <space> <symbol>) <newline>

>>> file begin
<nonterminal> <newline>
(<line>)*
>>> file end
```

### Dot File
```
<node>		[-_A-Za-z0-9]+
<label>		<terminal>
<newline>	\n
<line>		<node> "->" <node> "[" label "=" '"' <label> '"' "]" <newline>

>>> file begin
(<line>)*
>>> file end
```
