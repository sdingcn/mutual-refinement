#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <utility>

struct Grammar {
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
	std::unordered_map<long long, std::vector<int>> binaryRL;
	void addTerminal(int t);
	void addNonterminal(int nt);
	void addStartSymbol(int s);
	void addEmptyProduction(int l);
	void addUnaryProduction(int l, int r);
	void addBinaryProduction(int l, int r1, int r2);
	void initFastIndices();
};

#endif
