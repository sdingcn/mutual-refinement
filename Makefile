main : src/common.h src/grammar/grammar.cpp src/grammar/grammar.h src/graph/graph.cpp src/graph/graph.h src/main.cpp src/parser/parser.cpp src/parser/parser.h
	g++ -O3 -o main -Wall -Wextra -pedantic src/grammar/grammar.cpp src/parser/parser.cpp src/graph/graph.cpp src/main.cpp

.PHONY : clean
clean :
	-rm main
