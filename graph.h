#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"
#include "grammar.h"

struct Graph {
	int numberOfVertices;

	// edges
	vector<unordered_set<long long>> fastEdgeTest; // first vertex -> {FP(label, second vertex)}
	vector<vector<pair<int, int>>> adjacencyVector; // first vertex -> [(label, second vertex)]
	vector<vector<pair<int, int>>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]

	// records for reachability closures
	vector<unordered_map<int, unordered_set<int>>> negligibleRecord; // i -> j -> {negligible symbol}
	vector<unordered_map<int, unordered_set<int>>> unaryRecord; // i -> j -> {unary production number}
	vector<unordered_map<int, unordered_set<long long>>> binaryRecord; // i -> j -> {FP(binary prodution number, middle vertex)}

	Graph(int n);

	void addEdge(int i, int x, int j);

	void fillEdges(const vector<Edge> &edges);

	void fillEdges(const set<Edge> &edges);

	bool hasEdge(int i, int x, int j) const;

	bool runPureReachability(int i, int j) const;

	void runCFLReachability(const Grammar &g);

	set<Edge> getCFLReachabilityEdgeClosure(int i, int j, const Grammar &g) const;
};

#endif
