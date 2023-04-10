CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O3 -UALIAS
LDFLAGS = 

main: main.o grammar.o graph.o hasher.o
	$(CXX) $(LDFLAGS) main.o grammar.o graph.o hasher.o -o main
main.o: src/main.cpp src/grammar/grammar.h src/graph/graph.h src/hasher/hasher.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
grammar.o: src/grammar/grammar.cpp src/grammar/grammar.h src/hasher/hasher.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
graph.o: src/graph/graph.cpp src/graph/graph.h src/grammar/grammar.h src/hasher/hasher.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
hasher.o: src/hasher/hasher.cpp src/hasher/hasher.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY : clean
clean :
	-rm *.o main
