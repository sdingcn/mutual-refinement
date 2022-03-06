#include "graph.h"
#include <vector>
#include <deque>
#include <unordered_set>
#include <utility>
#include <tuple>
#include "../grammar/grammar.h"

void Graph::setNumberOfVertices(int n) {
	numberOfVertices = n;
	adjacencyVector.resize(n);
	counterAdjacencyVector.resize(n);
}

void Graph::addEdge(long long e) { // i --x--> j
	int i = fast_triple_first(e);
	int x = fast_triple_second(e);
	int j = fast_triple_third(e);
	fastEdgeTest.insert(e);
	adjacencyVector[i].push_back(make_fast_pair(x, j));
	counterAdjacencyVector[j].push_back(make_fast_pair(i, x));
}

void Graph::addEdges(const std::unordered_set<long long> &edges) {
	for (long long e : edges) {
		addEdge(e);
	}
}

bool Graph::hasEdge(long long e) const {
	return fastEdgeTest.count(e) == 1;
}

std::vector<long long> Graph::runCFLReachability(const Grammar &grammar) {
	std::vector<long long> startSummaries;
	std::deque<long long> w;
	for (long long e : fastEdgeTest) { // add all original edges to the worklist
		w.push_front(e);
	}
	for (int x : grammar.emptyProductions) {
		for (int i = 0; i < numberOfVertices; i++) {
			long long e = make_fast_triple(i, x, i);
			addEdge(e);
			w.push_front(e);
			if (x == grammar.startSymbol) {
				startSummaries.push_back(e);
			}
		}
	}
	while (!w.empty()) {
		long long e = w.front();
		w.pop_front();

		// i --y--> j
		int i = fast_triple_first(e);
		int y = fast_triple_second(e);
		int j = fast_triple_third(e);

		std::vector<long long> tba; // to be added

		for (int ind : grammar.unaryRL[y]) { // x -> y
			int x = grammar.unaryProductions[ind].first;
			tba.push_back(make_fast_triple(i, x, j));
		}
		for (long long zk : adjacencyVector[j]) { // x -> yz
			int z = fast_pair_first(zk);
			int k = fast_pair_second(zk);
			for (int ind : grammar.binaryRL[make_fast_pair(y, z)]) {
				int x = grammar.binaryProductions[ind].first;
				tba.push_back(make_fast_triple(i, x, k));
			}
		}
		for (long long kz : counterAdjacencyVector[i]) { // x -> zy
			int k = fast_pair_first(kz);
			int z = fast_pair_second(kz);
			for (int ind : grammar.binaryRL[make_fast_pair(z, y)]) {
				int x = grammar.binaryProductions[ind].first;
				tba.push_back(make_fast_triple(k, x, j));
			}
		}
		for (long long e1 : tba) {
			if (!hasEdge(e1)) {
				addEdge(e1);
				w.push_front(e1);
				if (fast_triple_second(e1) == grammar.startSymbol) {
					startSummaries.push_back(e1);
				}
			}
		}
	}
	return startSummaries;
}

std::unordered_set<long long> Graph::getEdgeClosure(const Grammar &grammar, const std::vector<long long> &startSummaries) const {
	std::unordered_set<long long> closure;
	std::unordered_set<long long> vis;
	std::deque<long long> w;
	for (long long e : startSummaries) {
		vis.insert(e);
		w.push_back(e);
	}
	while (!w.empty()) {
		long long e = w.front();
		w.pop_front();

		// i --x--> j
		int i = fast_triple_first(e);
		int x = fast_triple_second(e);
		int j = fast_triple_third(e);
		if (grammar.terminals.count(x) == 1) {
			closure.insert(e);
		}

		for (int ind : grammar.unaryLR[x]) { // x -> y
			long long e1 = make_fast_triple(i, grammar.unaryProductions[ind].second, j);
			if (hasEdge(e1) && vis.count(e1) == 0) {
				vis.insert(e1);
				w.push_back(e1);
			}
		}
		for (long long yk : adjacencyVector[i]) { // x -> yz
			// i --x--> j  =  i --y--> k  +  k --z--> j
			int y = fast_pair_first(yk);
			int k = fast_pair_second(yk);
			for (int ind : grammar.binaryLR[x]) {
				auto &p = grammar.binaryProductions[ind];
				if (p.second.first == y) {
					long long e1 = make_fast_triple(i, y, k);
					long long e2 = make_fast_triple(k, p.second.second, j);
					if (hasEdge(e2)) {
						if (vis.count(e1) == 0) {
							vis.insert(e1);
							w.push_back(e1);
						}
						if (vis.count(e2) == 0) {
							vis.insert(e2);
							w.push_back(e2);
						}
					}
				}
			}
		}
	}
	return closure;
}

void clear() {
	numberOfVertices = 0;
	fastEdgeTest.clear();
	adjacencyVector.clear();
	counterAdjacencyVector.clear();
}
