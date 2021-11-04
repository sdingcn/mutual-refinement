#include "grammar.h"
#include <vector>
#include <unordered_map>
#include <iostream>
#include <utility>

void Grammar::addTerminal(int t) {
	terminals.insert(t);
}

void Grammar::addNonterminal(int nt) {
	nonterminals.insert(nt);
}

void Grammar::addEmptyProduction(int l) {
	emptyProductions.push_back(l);
}

void Grammar::addUnaryProduction(int l, int r) {
	unaryProductions.push_back(std::make_pair(l, r));
}

void Grammar::addBinaryProduction(int l, int r1, int r2) {
	binaryProductions.push_back(std::make_pair(l, std::make_pair(r1, r2)));
}

void Grammar::addStartSymbol(int s) {
	startSymbol = s;
}

void Grammar::init(int total) {
	unaryProductionsInv = std::vector<std::vector<int>>(total);
	binaryProductionsInv = std::vector<std::unordered_map<int, std::vector<int>>>(total);
	int nu = unaryProductions.size();
	int nb = binaryProductions.size();
	for (int i = 0; i < nu; i++) {
		unaryProductionsInv[unaryProductions[i].second].push_back(i);
	}
	for (int i = 0; i < nb; i++) {
		binaryProductionsInv[binaryProductions[i].second.first][binaryProductions[i].second.second].push_back(i);
	}
}
