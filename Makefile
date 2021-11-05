main : src/common.h src/grammar/grammar.h src/grammar/grammar.cpp src/graph/graph.h src/graph/graph.cpp src/parser/parser.h src/parser/parser.cpp src/main.cpp
	g++ -DTRACE -O3 -o main -Wall -Wextra -pedantic src/grammar/grammar.cpp src/graph/graph.cpp src/parser/parser.cpp src/main.cpp

.PHONY : clean
clean :
	-rm main
