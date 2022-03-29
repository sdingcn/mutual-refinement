#include "../common.h"
#include "grammar.h"
#include <utility>

void Grammar::addTerminal(int t) {
	terminals.insert(t);
}

void Grammar::addNonterminal(int nt) {
	nonterminals.insert(nt);
}

void Grammar::addStartSymbol(int s) {
	startSymbol = s;
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

void Grammar::initFastIndices() {
	int ne = emptyProductions.size();
	for (int i = 0; i < ne; i++) {
		emptyLR[emptyProductions[i]].push_back(i);
	}
	int nu = unaryProductions.size();
	for (int i = 0; i < nu; i++) {
		unaryLR[unaryProductions[i].first].push_back(i);
		unaryRL[unaryProductions[i].second].push_back(i);
	}
	int nb = binaryProductions.size();
	for (int i = 0; i < nb; i++) {
		binaryLR[binaryProductions[i].first].push_back(i);
		binaryRL[make_fast_pair(binaryProductions[i].second.first, binaryProductions[i].second.second)].push_back(i);
	}
}
