files = src/common.h src/die.h src/die.cpp src/grammar/grammar.h src/grammar/grammar.cpp src/graph/graph.h src/graph/graph.cpp src/parser/parser.h src/parser/parser.cpp src/main.cpp

cppfiles = src/die.cpp src/grammar/grammar.cpp src/graph/graph.cpp src/parser/parser.cpp src/main.cpp

cppflags = -O3 -Wall -Wextra -pedantic

all : naive refine

naive : $(files)
	g++ -DNAIVE -o naive $(cppflags) $(cppfiles)

refine : $(files)
	g++ -DREFINE -o refine $(cppflags) $(cppfiles)

robin : robin-naive robin-refine

robin-naive : $(files)
	g++ -DROBIN -DNAIVE -o robin-naive $(cppflags) $(cppfiles)

robin-refine : $(files)
	g++ -DROBIN -DREFINE -o robin-refine $(cppflags) $(cppfiles)

.PHONY : clean
clean :
	-rm naive refine robin-naive robin-refine
