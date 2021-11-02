#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <utility>

struct Grammar {
	std::unordered_set<int> terminals;
	std::unordered_set<int> nonterminals;
	std::vector<int> emptyProductions;
	std::vector<std::pair<int, int>> unaryProductions;
	std::vector<std::pair<int, std::pair<int, int>>> binaryProductions;
	int startSymbol;
	std::vector<std::vector<int>> unaryProductionsInv; // RHS symbol -> {corresponding indices in unaryProductions}
	std::vector<std::unordered_map<int, std::vector<int>>> binaryProductionsInv; // RHS symbol 1 -> RHS symbol 2 -> {corresponding indices in binaryProductions}
	void init(int total); // total is the max number used to encode labels
};

#endif
