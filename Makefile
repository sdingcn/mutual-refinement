files = src/common.h src/grammar/grammar.h src/grammar/grammar.cpp src/graph/graph.h src/graph/graph.cpp src/parser/parser.h src/parser/parser.cpp src/main.cpp

cppfiles = src/grammar/grammar.cpp src/graph/graph.cpp src/parser/parser.cpp src/main.cpp

cppflags = -O3 -Wall -Wextra -pedantic

all : naive refine graph

naive : $(files)
	g++ -DNAIVE -o naive $(cppflags) $(cppfiles)

refine : $(files)
	g++ -DREFINE -o refine $(cppflags) $(cppfiles)

graph : $(files)
	g++ -DGRAPH -o graph $(cppflags) $(cppfiles)

.PHONY : clean
clean :
	-rm naive refine graph
