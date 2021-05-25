#include "graph.h"
#include <vector>
#include <set>
#include <deque>
#include <unordered_set>
#include <utility>
#include <iostream>
#include "../grammar/grammar.h"

Graph::Graph(int n) : numberOfVertices(n), fastEdgeTest(n), adjacencyVector(n), counterAdjacencyVector(n),
                       unaryRecord(n), binaryRecord(n) {}

void Graph::addEdge(int i, int x, int j) { // i --x--> j
	fastEdgeTest[i].insert(make_fast_pair(x, j));
	adjacencyVector[i].push_back(std::make_pair(x, j));
	counterAdjacencyVector[j].push_back(std::make_pair(i, x));
}

void Graph::fillEdges(const std::vector<Edge> &edges) {
	for (auto &ijs : edges) {
		addEdge(ijs.first.first, ijs.second, ijs.first.second);
	}
}

void Graph::fillEdges(const std::set<Edge> &edges) {
	for (auto &ijs : edges) {
		addEdge(ijs.first.first, ijs.second, ijs.first.second);
	}
}

bool Graph::hasEdge(int i, int x, int j) const {
	return fastEdgeTest[i].count(make_fast_pair(x, j)) == 1;
}

bool Graph::runPureReachability(int i, int j) const {
	std::deque<int> q;
	std::unordered_set<int> s;
	q.push_back(i);
	s.insert(i);
	while (!q.empty()) {
		int c = q.front();
		q.pop_front();
		if (c == j) {
			return true;
		}
		for (auto &sj : adjacencyVector[c]) {
			if (s.count(sj.second) == 0) {
				q.push_back(sj.second);
				s.insert(sj.second);
			}
		}
	}
	return false;
}

void Graph::runCFLReachability(const Grammar &g) {
	std::deque<Edge> w; // ((first vertex, second vertex), label)
	for (int i = 0; i < numberOfVertices; i++) { // add all edges to the worklist
		for (auto &sj : adjacencyVector[i]) { // --s--> j
			w.push_back(make_edge(i, sj.first, sj.second));
		}
	}
	int nep = g.emptyProductions.size();
	for (int ind = 0; ind < nep; ind++) { // add empty edges to the edge set and the worklist
		int x = g.emptyProductions[ind];
		for (int i = 0; i < numberOfVertices; i++) {
			addEdge(i, x, i);
			w.push_back(make_edge(i, x, i));
		}
	}
	while (!w.empty()) {
		Edge e = w.front();
		w.pop_front();

		// i --y--> j
		int i = e.first.first, j = e.first.second, y = e.second;

		std::vector<Edge> tba;

		for (int ind : g.unaryProductionsInv[y]) { // x -> y
			auto &p = g.unaryProductions[ind];
			int x = p.first;
			unaryRecord[i][j].insert(ind);
			if (!hasEdge(i, x, j)) {
				tba.push_back(make_edge(i, x, j));
			}
		}
		for (int ind : g.binaryProductionsFirstInv[y]) { // x -> yz
			auto &p = g.binaryProductions[ind];
			int x = p.first, z = p.second.second;
			for (auto &sk : adjacencyVector[j]) { // --s--> k
				if (sk.first == z) { // --z--> k
					int k = sk.second;
					binaryRecord[i][k].insert(make_fast_pair(ind, j));
					if (!hasEdge(i, x, k)) {
						tba.push_back(make_edge(i, x, k));
					}
				}
			}
		}
		for (int ind : g.binaryProductionsSecondInv[y]) { // x -> zy
			auto &p = g.binaryProductions[ind];
			int x = p.first, z = p.second.first;
			for (auto &ks : counterAdjacencyVector[i]) {
				if (ks.second == z) { // k --z-->
					int k = ks.first;
					binaryRecord[k][j].insert(make_fast_pair(ind, i));
					if (!hasEdge(k, x, j)) {
						tba.push_back(make_edge(k, x, j));
					}
				}
			}
		}
		for (auto &e : tba) {
			addEdge(e.first.first, e.second, e.first.second);
			w.push_back(e);
		}
	}
}

std::set<Edge> Graph::getCFLReachabilityEdgeClosure(int i, int j, const Grammar &g) const {
	std::set<Edge> closure;
	std::set<Edge> vis;
	std::deque<Edge> q;
	if (hasEdge(i, g.startSymbol, j)) {
		Edge start = make_edge(i, g.startSymbol, j);
		vis.insert(start);
		q.push_back(start);
	}
	while (!q.empty()) { // BFS
		Edge cur = q.front();
		q.pop_front();

		// i --x--> j
		int i = cur.first.first, j = cur.first.second, x = cur.second;
		if (g.terminals.count(x) == 1) {
			closure.insert(cur);
		}

		if (unaryRecord[i].count(j) == 1) {
			for (int ind : unaryRecord[i].at(j)) {
				if (g.unaryProductions[ind].first == x) {
					Edge nxt = make_edge(i, g.unaryProductions[ind].second, j);
					if (vis.count(nxt) == 0) {
						vis.insert(nxt);
						q.push_back(nxt);
					}
				}
			}
		}
		if (binaryRecord[i].count(j) == 1) {
std::cerr << "E\n";
			for (long long fp : binaryRecord[i].at(j)) {
				auto p = unpack_fast_pair(fp);
				int ind = p.first, k = p.second; // i --> k --> j
				if (g.binaryProductions[ind].first == x) {
					Edge nxt1 = make_edge(i, g.binaryProductions[ind].second.first, k);
					Edge nxt2 = make_edge(k, g.binaryProductions[ind].second.second, j);
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
std::cerr << "F\n";
		}
	}
	return closure;
}
