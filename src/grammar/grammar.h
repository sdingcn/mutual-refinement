#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <unordered_set>
#include <vector>
#include <utility>

struct Grammar {
	std::unordered_set<int> terminals;
	std::unordered_set<int> nonterminals;
	std::vector<int> emptyProductions;
	std::vector<std::pair<int, int>> unaryProductions;
	std::vector<std::pair<int, std::pair<int, int>>> binaryProductions;
	int startSymbol;
	std::vector<std::vector<int>> unaryProductionsInv; // right hand side symbol -> {corresponding indices in unaryProductions}
	std::vector<std::vector<int>> binaryProductionsFirstInv; // right hand side symbol 1 -> {corresponding indices in binaryProductions}
	std::vector<std::vector<int>> binaryProductionsSecondInv; // right hand side symbol 2 -> {corresponding indices in binaryProductions}
	void fillInv(int total);
};

#endif
