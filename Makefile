main : common.h grammar.cpp grammar.h graph.cpp graph.h main.cpp parser.cpp parser.h
	g++ -O3 -o main -Wall -Wextra -pedantic grammar.cpp parser.cpp graph.cpp main.cpp

.PHONY : clean
clean :
	-rm main
