#ifndef GRAPH_H
#define GRAPH_H

#include "../hasher/hasher.h"
#include "../grammar/grammar.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct Graph {
	using Edge = std::tuple<int, int, int>;
	using EdgeHasher = IntTripleHasher;
	std::unordered_set<Edge, EdgeHasher> fastEdgeTest;
	std::vector<std::vector<std::pair<int, int>>> adjacencyVector; // first vertex -> [(label, second vertex)]
	std::vector<std::vector<std::pair<int, int>>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]
	void reinit(int n, const std::unordered_set<Edge, EdgeHasher> &edges);
	void addEdge(const Edge &e);
	bool hasEdge(const Edge &e) const;
	std::vector<Edge> runCFLReachability(
		const Grammar &grammar,
		const bool trace,
		std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> &record);
	std::unordered_set<Edge, EdgeHasher> getEdgeClosure(
		const Grammar &grammar,
		const std::vector<Edge> &startSummaries,
		const std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> &record) const;
};

#endif
