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
	// RHS symbol -> {corresponding indices in unaryProductions}
	std::vector<std::vector<int>> unaryProductionsInv;
	// RHS symbol 1 -> RHS symbol 2 -> {corresponding indices in binaryProductions}
	std::vector<std::unordered_map<int, std::vector<int>>> binaryProductionsInv;
	void addTerminal(int t);
	void addNonterminal(int nt);
	void addEmptyProduction(int l);
	void addUnaryProduction(int l, int r);
	void addBinaryProduction(int l, int r1, int r2);
	void addStartSymbol(int s);
	void init(int total); // total is the max number used to encode labels
};

#endif
