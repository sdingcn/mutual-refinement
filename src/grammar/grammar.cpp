#include "grammar.h"
#include <vector>

void Grammar::fillInv(int total) {
	unaryProductionsInv = std::vector<std::vector<int>>(total);
	binaryProductionsFirstInv = std::vector<std::vector<int>>(total);
	binaryProductionsSecondInv = std::vector<std::vector<int>>(total);
	int nu = unaryProductions.size();
	int nb = binaryProductions.size();
	for (int i = 0; i < nu; i++) {
		unaryProductionsInv[unaryProductions[i].second].push_back(i);
	}
	for (int i = 0; i < nb; i++) {
		binaryProductionsFirstInv[binaryProductions[i].second.first].push_back(i);
		binaryProductionsSecondInv[binaryProductions[i].second.second].push_back(i);
	}
}
