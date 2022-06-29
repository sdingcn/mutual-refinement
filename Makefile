files = src/grammar/grammar.h src/grammar/grammar.cpp src/graph/graph.h src/graph/graph.cpp src/hasher/hasher.h src/hasher/hasher.cpp src/parser/parser.h src/parser/parser.cpp src/main.cpp

cppfiles = src/grammar/grammar.cpp src/graph/graph.cpp src/hasher/hasher.cpp src/parser/parser.cpp src/main.cpp

cppflags = -O3 -Wall -Wextra -pedantic

all : naive refine

naive : $(files)
	g++ -std=c++11 -o naive $(cppflags) $(cppfiles)

refine : $(files)
	g++ -std=c++11 -DREFINE -o refine $(cppflags) $(cppfiles)

.PHONY : clean
clean :
	-rm naive refine
