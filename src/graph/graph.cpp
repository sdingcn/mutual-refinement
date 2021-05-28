#include "graph.h"
#include <vector>
#include <set>
#include <deque>
#include <unordered_set>
#include <utility>
#include <tuple>
#include "../grammar/grammar.h"

Graph::Graph(const Grammar &g, int n) : grammar(g), numberOfVertices(n), fastEdgeTest(n), adjacencyVector(n), counterAdjacencyVector(n),
                       unaryRecord(n), binaryRecord(n) {}

void Graph::addEdge(const Edge &e) { // i --x--> j
	int i = std::get<0>(e);
	int x = std::get<1>(e);
	int j = std::get<2>(e);
	fastEdgeTest[i].insert(make_fast_pair(x, j));
	adjacencyVector[i].push_back(std::make_pair(x, j));
	counterAdjacencyVector[j].push_back(std::make_pair(i, x));
}

void Graph::fillEdges(const std::vector<Edge> &edges) {
	for (auto &e : edges) {
		addEdge(e);
	}
}

void Graph::fillEdges(const std::set<Edge> &edges) {
	for (auto &e : edges) {
		addEdge(e);
	}
}

bool Graph::hasEdge(const Edge &e) const {
	int i = std::get<0>(e);
	int x = std::get<1>(e);
	int j = std::get<2>(e);
	return fastEdgeTest[i].count(make_fast_pair(x, j)) == 1;
}

void Graph::runCFLReachability() {
	std::deque<Edge> w; // ((first vertex, second vertex), label)
	for (int i = 0; i < numberOfVertices; i++) { // add all edges to the worklist
		for (auto &sj : adjacencyVector[i]) { // --s--> j
			Edge e = std::make_tuple(i, sj.first, sj.second);
			w.push_back(e);
		}
	}
	int nep = grammar.emptyProductions.size();
	for (int ind = 0; ind < nep; ind++) { // add empty edges to the edge set and the worklist
		int x = grammar.emptyProductions[ind];
		for (int i = 0; i < numberOfVertices; i++) {
			Edge e = std::make_tuple(i, x, i);
			addEdge(e);
			w.push_back(e);
		}
	}
	while (!w.empty()) {
		Edge e = w.front();
		w.pop_front();

		// i --y--> j
		int i = std::get<0>(e);
		int y = std::get<1>(e);
		int j = std::get<2>(e);

		std::vector<Edge> tba;

		for (int ind : grammar.unaryProductionsInv[y]) { // x -> y
			auto &p = grammar.unaryProductions[ind];
			int x = p.first;
			unaryRecord[i][j].insert(ind);
			Edge e = std::make_tuple(i, x, j);
			if (!hasEdge(e)) {
				tba.push_back(e);
			}
		}
		// TODO: not using grammars
		for (int ind : grammar.binaryProductionsFirstInv[y]) { // x -> yz
			auto &p = grammar.binaryProductions[ind];
			int x = p.first, z = p.second.second;
			for (auto &sk : adjacencyVector[j]) { // --s--> k
				if (sk.first == z) { // --z--> k
					int k = sk.second;
					binaryRecord[i][k].insert(make_fast_pair(ind, j));
					Edge e = std::make_tuple(i, x, k);
					if (!hasEdge(e)) {
						tba.push_back(e);
					}
				}
			}
		}
		for (int ind : grammar.binaryProductionsSecondInv[y]) { // x -> zy
			auto &p = grammar.binaryProductions[ind];
			int x = p.first, z = p.second.first;
			for (auto &ks : counterAdjacencyVector[i]) {
				if (ks.second == z) { // k --z-->
					int k = ks.first;
					binaryRecord[k][j].insert(make_fast_pair(ind, i));
					Edge e = std::make_tuple(k, x, j);
					if (!hasEdge(e)) {
						tba.push_back(e);
					}
				}
			}
		}
		for (auto &e : tba) {
			addEdge(e);
			w.push_back(e);
		}
	}
}

std::set<Edge> Graph::getCFLReachabilityEdgeClosure(int i, int j) const {
	std::set<Edge> closure;
	std::set<Edge> vis;
	std::deque<Edge> q;
	Edge start = std::make_tuple(i, grammar.startSymbol, j);
	if (hasEdge(start)) {
		vis.insert(start);
		q.push_back(start);
	}
	while (!q.empty()) { // BFS
		Edge e = q.front();
		q.pop_front();

		// i --x--> j
		int i = std::get<0>(e);
		int x = std::get<1>(e);
		int j = std::get<2>(e);
		if (grammar.terminals.count(x) == 1) {
			closure.insert(e);
		}

		if (unaryRecord[i].count(j) == 1) {
			// TODO: not exhaustive?
			for (int ind : unaryRecord[i].at(j)) {
				if (grammar.unaryProductions[ind].first == x) {
					Edge nxt = std::make_tuple(i, grammar.unaryProductions[ind].second, j);
					if (vis.count(nxt) == 0) {
						vis.insert(nxt);
						q.push_back(nxt);
					}
				}
			}
		}
		if (binaryRecord[i].count(j) == 1) {
			for (long long fp : binaryRecord[i].at(j)) {
				auto p = unpack_fast_pair(fp);
				int ind = p.first, k = p.second; // i --> k --> j
				if (grammar.binaryProductions[ind].first == x) {
					Edge nxt1 = std::make_tuple(i, grammar.binaryProductions[ind].second.first, k);
					Edge nxt2 = std::make_tuple(k, grammar.binaryProductions[ind].second.second, j);
					if (vis.count(nxt1) == 0) {
						vis.insert(nxt1);
						q.push_back(nxt1);
					}
					if (vis.count(nxt2) == 0) {
						vis.insert(nxt2);
						q.push_back(nxt2);
					}
				}
			}
		}
	}
	return closure;
}
