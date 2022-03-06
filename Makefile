files = src/common.h src/die.h src/die.cpp src/grammar/grammar.h src/grammar/grammar.cpp src/graph/graph.h src/graph/graph.cpp src/parser/parser.h src/parser/parser.cpp src/main.cpp

cppfiles = src/die.cpp src/grammar/grammar.cpp src/graph/graph.cpp src/parser/parser.cpp src/main.cpp

cppflags = -O3 -Wall -Wextra -pedantic

main : $(files)
	g++ -o main $(cppflags) $(cppfiles)

.PHONY : clean
clean :
	-rm main
