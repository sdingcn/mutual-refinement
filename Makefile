main : mutual-refinement/common.h mutual-refinement/grammar/grammar.h mutual-refinement/grammar/grammar.cpp mutual-refinement/graph/graph.h mutual-refinement/graph/graph.cpp mutual-refinement/parser/parser.h mutual-refinement/parser/parser.cpp mutual-refinement/main.cpp
	g++ -O3 -o main -Wall -Wextra -pedantic mutual-refinement/grammar/grammar.cpp mutual-refinement/graph/graph.cpp mutual-refinement/parser/parser.cpp mutual-refinement/main.cpp

.PHONY : clean
clean :
	-rm main
