#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "../common.h"
#include "../grammar/grammar.h"

class Graph {
	const Grammar &grammar;
	int numberOfVertices;

	// edges
	std::vector<std::unordered_set<long long>> fastEdgeTest; // first vertex -> {(label, second vertex)}
	std::vector<std::vector<long long>> adjacencyVector; // first vertex -> [(label, second vertex)]
	std::vector<std::vector<long long>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]

	// records for reachability closures
	std::vector<std::unordered_map<int, std::unordered_set<int>>> unaryRecord; // i -> j -> {unary production number}
	std::vector<std::unordered_map<int, std::unordered_set<long long>>> binaryRecord; // i -> j -> {(binary prodution number, middle vertex)}
public:
	Graph(const Grammar &g, int n);

	void addEdge(long long e);

	void fillEdges(const std::vector<long long> &edges);

	void fillEdges(const std::unordered_set<long long> &edges);

	bool hasEdge(long long e) const;

	void runCFLReachability();

	std::unordered_set<long long> getCFLReachabilityEdgeClosure(int i, int j) const;
};

#endif
