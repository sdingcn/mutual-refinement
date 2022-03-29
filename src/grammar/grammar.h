#ifndef GRAMMAR_H
#define GRAMMAR_H

#ifdef ROBIN
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
template <typename K, typename V> using hash_map = tsl::robin_map<K, V>;
template <typename T> using hash_set = tsl::robin_set<T>;
#else
#include <unordered_map>
#include <unordered_set>
template <typename K, typename V> using hash_map = std::unordered_map<K, V>;
template <typename T> using hash_set = std::unordered_set<T>;
#endif
#include <vector>
#include <utility>

struct Grammar {
	hash_set<int> terminals;
	hash_set<int> nonterminals;
	int startSymbol;
	std::vector<int> emptyProductions;
	std::vector<std::pair<int, int>> unaryProductions;
	std::vector<std::pair<int, std::pair<int, int>>> binaryProductions;
	// fast index access
	hash_map<int, std::vector<int>> emptyLR;
	hash_map<int, std::vector<int>> unaryLR;
	hash_map<int, std::vector<int>> unaryRL;
	hash_map<int, std::vector<int>> binaryLR;
	hash_map<long long, std::vector<int>> binaryRL;
	void addTerminal(int t);
	void addNonterminal(int nt);
	void addStartSymbol(int s);
	void addEmptyProduction(int l);
	void addUnaryProduction(int l, int r);
	void addBinaryProduction(int l, int r1, int r2);
	void initFastIndices();
};

#endif
