#ifndef GRAPH_H
#define GRAPH_H

#include "../hasher/hasher.h"
#include "../grammar/grammar.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct Graph {
	std::unordered_set<Edge, EdgeHasher> fastEdgeTest;
	std::vector<std::vector<std::pair<int, int>>> adjacencyVector; // v1 -> [(sym, v2)]
	std::vector<std::vector<std::pair<int, int>>> counterAdjacencyVector; // v2 -> [(v1, sym)]
	void reinit(int n, const std::unordered_set<Edge, EdgeHasher> &edges);
	void addEdge(const Edge &e);
	bool hasEdge(const Edge &e) const;
	std::unordered_set<Edge, EdgeHasher> runCFLReachability(
		const Grammar &grammar,
		const bool trace,
		std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> &record);
	std::unordered_set<Edge, EdgeHasher> getEdgeClosure(
		const Grammar &grammar,
		const std::unordered_set<Edge, EdgeHasher> &result,
		const std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> &record) const;
};

#endif
