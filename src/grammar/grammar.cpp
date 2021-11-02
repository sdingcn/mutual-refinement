#include "grammar.h"
#include <vector>
#include <unordered_map>
#include <iostream>

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
