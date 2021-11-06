#include "graph.h"
#include <vector>
#include <set>
#include <deque>
#include <unordered_set>
#include <utility>
#include <tuple>
#include "../grammar/grammar.h"

Graph::Graph(const Grammar &g, int n) :
	grammar(g),
	numberOfVertices(n),
	adjacencyVector(n),
	counterAdjacencyVector(n),
	unaryRecord(n),
	binaryRecord(n) {}

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

void Graph::fillEdges(const std::vector<long long> &edges) {
	for (long long e : edges) {
		addEdge(e);
	}
}

void Graph::fillEdges(const std::unordered_set<long long> &edges) {
	for (long long e : edges) {
		addEdge(e);
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
			long long e = make_fast_triple(i, x, j);
			if (!hasEdge(e)) {
				tba.push_back(e);
			}
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
					long long e = make_fast_triple(i, x, k);
					if (!hasEdge(e)) {
						tba.push_back(e);
					}
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
					long long e = make_fast_triple(k, x, j);
					if (!hasEdge(e)) {
						tba.push_back(e);
					}
				}
			}
		}
		for (long long e : tba) {
			addEdge(e);
			w.push_front(e);
		}
	}
}

std::unordered_set<long long> Graph::getCFLReachabilityEdgeClosure(bool all, int i, int j) const {
	std::unordered_set<long long> closure;
	std::unordered_set<long long> vis;
	std::deque<long long> q;
	if (all == false) {
		long long start = make_fast_triple(i, grammar.startSymbol, j);
		if (hasEdge(start)) {
			vis.insert(start);
			q.push_back(start);
		}
	} else {
		for (long long start : startSummaries) {
			vis.insert(start);
			q.push_back(start);
		}
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
					long long nxt = make_fast_triple(i, grammar.unaryProductions[ind].second, j);
					if (vis.count(nxt) == 0) {
						vis.insert(nxt);
						q.push_back(nxt);
					}
				}
			}
		}
		if (binaryRecord[i].count(j) == 1) {
			for (long long ind_k : binaryRecord[i].at(j)) {
				int ind = fast_pair_first(ind_k), k = fast_pair_second(ind_k);
				if (grammar.binaryProductions[ind].first == x) {
					long long nxt1 = make_fast_triple(i, grammar.binaryProductions[ind].second.first, k);
					long long nxt2 = make_fast_triple(k, grammar.binaryProductions[ind].second.second, j);
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
