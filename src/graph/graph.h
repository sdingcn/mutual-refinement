#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "../common.h"
#include "../grammar/grammar.h"

struct Graph {
	int numberOfVertices = 0;
	std::unordered_set<long long> fastEdgeTest; // {(first vertex, label, second vertex)}
	std::vector<std::vector<long long>> adjacencyVector; // first vertex -> [(label, second vertex)]
	std::vector<std::vector<long long>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]
	void setNumberOfVertices(int n);
	void addEdge(long long e);
	void addEdges(const std::unordered_set<long long> &edges);
	bool hasEdge(long long e) const;
	std::vector<long long> runCFLReachability(
		const Grammar &grammar,
		const bool trace,
		std::unordered_map<long long, std::unordered_set<long long>> &record);
	std::unordered_set<long long> getEdgeClosure(
		const Grammar &grammar,
		const std::vector<long long> &startSummaries,
		const std::unordered_map<long long, std::unordered_set<long long>> &record) const;
	void clear();
};

#endif
