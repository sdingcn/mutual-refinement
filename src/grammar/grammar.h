#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "../common.h"

struct Grammar {
	unordered_set<int> terminals;
	unordered_set<int> nonterminals;
	vector<int> emptyProductions;
	vector<pair<int, int>> unaryProductions;
	vector<pair<int, pair<int, int>>> binaryProductions;
	int startSymbol;
	vector<vector<int>> unaryProductionsInv; // right hand side symbol -> {corresponding indices in unaryProductions}
	vector<vector<int>> binaryProductionsFirstInv; // right hand side symbol 1 -> {corresponding indices in binaryProductions}
	vector<vector<int>> binaryProductionsSecondInv; // right hand side symbol 2 -> {corresponding indices in binaryProductions}
	void fillInv(int total);
};

#endif
