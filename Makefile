files = src/error/error.h src/error/error.cpp src/grammar/grammar.h src/grammar/grammar.cpp src/graph/graph.h src/graph/graph.cpp src/hasher/hasher.h src/hasher/hasher.cpp src/parser/parser.h src/parser/parser.cpp src/main.cpp

cppfiles = src/error/error.cpp src/grammar/grammar.cpp src/graph/graph.cpp src/hasher/hasher.cpp src/parser/parser.cpp src/main.cpp

cppflags = -O3 -Wall -Wextra -pedantic

all : naive refine

naive : $(files)
	g++ -DNAIVE -o naive $(cppflags) $(cppfiles)

refine : $(files)
	g++ -DREFINE -o refine $(cppflags) $(cppfiles)

.PHONY : clean
clean :
	-rm naive refine
