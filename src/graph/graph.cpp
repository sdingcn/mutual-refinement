#include "graph.h"
#include <vector>
#include <deque>
#include <unordered_set>
#include <utility>
#include <tuple>
#include "../grammar/grammar.h"

Graph::Graph(const Grammar &g, int n, const std::unordered_set<long long> &edges)
	: grammar(g),
	numberOfVertices(n),
	adjacencyVector(n),
	counterAdjacencyVector(n),
	unaryRecord(n),
	binaryRecord(n) {
	for (long long e : edges) {
		addEdge(e);
	}
}

void Graph::addEdge(long long e) { // i --x--> j
	int i = fast_triple_first(e);
	int x = fast_triple_second(e);
	int j = fast_triple_third(e);
	fastEdgeTest.insert(e);
	adjacencyVector[i].push_back(make_fast_pair(x, j));
	counterAdjacencyVector[j].push_back(make_fast_pair(i, x));
	if (x == grammar.startSymbol) {
		startSummaries.push_back(e);
	}
}

bool Graph::hasEdge(long long e) const {
	return fastEdgeTest.count(e) == 1;
}

void Graph::runCFLReachability() {
	std::deque<long long> w;
	for (int i = 0; i < numberOfVertices; i++) { // add all original edges to the worklist
		for (long long sj : adjacencyVector[i]) { // --s--> j
			w.push_front(make_fast_triple(i, fast_pair_first(sj), fast_pair_second(sj)));
		}
	}
	int nep = grammar.emptyProductions.size();
	for (int ind = 0; ind < nep; ind++) { // add empty edges to the edge set and the worklist
		int x = grammar.emptyProductions[ind];
		for (int i = 0; i < numberOfVertices; i++) {
			long long e = make_fast_triple(i, x, i);
			addEdge(e);
			w.push_front(e);
		}
	}
	while (!w.empty()) {
		long long e = w.front();
		w.pop_front();

		// i --y--> j
		int i = fast_triple_first(e);
		int y = fast_triple_second(e);
		int j = fast_triple_third(e);

		std::vector<long long> tba;

		for (int ind : grammar.unaryProductionsInv[y]) { // x -> y
#ifndef NAIVE
			unaryRecord[i][j].insert(ind);
#endif
			int x = grammar.unaryProductions[ind].first;
			tba.push_back(make_fast_triple(i, x, j));
		}
		for (long long zk : adjacencyVector[j]) { // x -> yz
			int z = fast_pair_first(zk);
			int k = fast_pair_second(zk);
			if (grammar.binaryProductionsInv[y].count(z) > 0) {
				for (int ind : grammar.binaryProductionsInv[y].at(z)) {
#ifndef NAIVE
					binaryRecord[i][k].insert(make_fast_pair(ind, j));
#endif
					int x = grammar.binaryProductions[ind].first;
					tba.push_back(make_fast_triple(i, x, k));
				}
			}
		}
		for (long long kz : counterAdjacencyVector[i]) { // x -> zy
			int k = fast_pair_first(kz);
			int z = fast_pair_second(kz);
			if (grammar.binaryProductionsInv[z].count(y) > 0) {
				for (int ind : grammar.binaryProductionsInv[z].at(y)) {
#ifndef NAIVE
					binaryRecord[k][j].insert(make_fast_pair(ind, i));
#endif
					int x = grammar.binaryProductions[ind].first;
					tba.push_back(make_fast_triple(k, x, j));
				}
			}
		}
		for (long long e1 : tba) {
			if (!hasEdge(e1)) {
				addEdge(e1);
				w.push_front(e1);
			}
		}
	}
}

std::unordered_set<long long> Graph::getCFLReachabilityEdgeClosure() const {
	std::unordered_set<long long> closure;
	std::unordered_set<long long> vis;
	std::deque<long long> q;
	for (long long start : startSummaries) {
		vis.insert(start);
		q.push_back(start);
	}
	while (!q.empty()) { // BFS
		long long e = q.front();
		q.pop_front();

		// i --x--> j
		int i = fast_triple_first(e);
		int x = fast_triple_second(e);
		int j = fast_triple_third(e);
		if (grammar.terminals.count(x) == 1) {
			closure.insert(e);
		}

		if (unaryRecord[i].count(j) == 1) {
			for (int ind : unaryRecord[i].at(j)) {
				if (grammar.unaryProductions[ind].first == x) {
					long long e1 = make_fast_triple(i, grammar.unaryProductions[ind].second, j);
					if (vis.count(e1) == 0) {
						vis.insert(e1);
						q.push_back(e1);
					}
				}
			}
		}
		if (binaryRecord[i].count(j) == 1) {
			for (long long ind_k : binaryRecord[i].at(j)) {
				int ind = fast_pair_first(ind_k), k = fast_pair_second(ind_k);
				if (grammar.binaryProductions[ind].first == x) {
					long long e1 = make_fast_triple(i, grammar.binaryProductions[ind].second.first, k);
					long long e2 = make_fast_triple(k, grammar.binaryProductions[ind].second.second, j);
					if (vis.count(e1) == 0) {
						vis.insert(e1);
						q.push_back(e1);
					}
					if (vis.count(e2) == 0) {
						vis.insert(e2);
						q.push_back(e2);
					}
				}
			}
		}
	}
	return closure;
}
