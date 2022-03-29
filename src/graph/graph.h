#ifndef GRAPH_H
#define GRAPH_H

#include "../common.h"
#include "../grammar/grammar.h"
#include <vector>
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

struct EdgeSet {
	std::vector<hash_set<long long>> s;
	void init(int n) {
		s.clear();
		s.resize(n);
	}
	void add(int i, int x, int j) {
		s[i].insert(make_fast_pair(x, j));
	}
	void add(int i, long long xj) {
		s[i].insert(xj);
	}
	void add(long long e) {
		s[fast_triple_first(e)].insert(fast_triple_tail(e));
	}
	bool has(int i, int x, int j) const {
		return s[i].count(make_fast_pair(x, j)) == 1;
	}
	bool has(int i, long long xj) const {
		return s[i].count(xj) == 1;
	}
	bool has(long long e) const {
		return s[fast_triple_first(e)].count(fast_triple_tail(e)) == 1;
	}
};

struct Graph {
	int numberOfVertices = 0;
	EdgeSet fastEdgeTest;
	std::vector<std::vector<long long>> adjacencyVector; // first vertex -> [(label, second vertex)]
	std::vector<std::vector<long long>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]
	void init(int n);
	void addEdge(long long e);
	void addEdges(const hash_set<long long> &edges);
	bool hasEdge(long long e) const;
	std::vector<long long> runCFLReachability(
		const Grammar &grammar,
		const bool trace,
		hash_map<long long, hash_set<long long>> &record);
	hash_set<long long> getEdgeClosure(
		const Grammar &grammar,
		const std::vector<long long> &startSummaries,
		const hash_map<long long, hash_set<long long>> &record) const;
};

#endif
