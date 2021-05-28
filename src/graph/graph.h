#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include "../common.h"
#include "../grammar/grammar.h"

class Graph {
	const Grammar &grammar;
	int numberOfVertices;

	// edges
	std::vector<std::unordered_set<long long>> fastEdgeTest; // first vertex -> {FP(label, second vertex)}
	std::vector<std::vector<std::pair<int, int>>> adjacencyVector; // first vertex -> [(label, second vertex)]
	std::vector<std::vector<std::pair<int, int>>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]

	// records for reachability closures
	std::vector<std::unordered_map<int, std::unordered_set<int>>> unaryRecord; // i -> j -> {unary production number}
	std::vector<std::unordered_map<int, std::unordered_set<long long>>> binaryRecord; // i -> j -> {FP(binary prodution number, middle vertex)}
public:
	Graph(const Grammar &g, int n);

	void addEdge(int i, int x, int j);

	void fillEdges(const std::vector<Edge> &edges);

	void fillEdges(const std::set<Edge> &edges);

	bool hasEdge(int i, int x, int j) const;

	// bool runPureReachability(int i, int j) const;

	void runCFLReachability();

	std::set<Edge> getCFLReachabilityEdgeClosure(int i, int j) const;
};

#endif
