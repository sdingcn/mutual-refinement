#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "../hasher/hasher.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

struct Grammar {
	void addTerminal(int t);
	void addNonterminal(int nt);
	void addStartSymbol(int s);
	void addEmptyProduction(int l);
	void addUnaryProduction(int l, int r);
	void addBinaryProduction(int l, int r1, int r2);
	void initFastIndices();
	// grammar contents
	std::unordered_set<int> terminals;
	std::unordered_set<int> nonterminals;
	int startSymbol;
	std::vector<int> emptyProductions;
	std::vector<std::pair<int, int>> unaryProductions;
	std::vector<std::pair<int, std::pair<int, int>>> binaryProductions;
	// fast index access
	std::unordered_map<int, std::vector<int>> emptyLR;
	std::unordered_map<int, std::vector<int>> unaryLR;
	std::unordered_map<int, std::vector<int>> unaryRL;
	std::unordered_map<int, std::vector<int>> binaryLR;
	std::unordered_map<std::pair<int, int>, std::vector<int>, IntPairHasher> binaryRL;
};

#endif
