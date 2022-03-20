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

std::vector<long long> Graph::runCFLReachability(
	const Grammar &grammar,
	const bool trace,
	std::unordered_map<long long, std::unordered_set<long long>> &record) {
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

		if (grammar.unaryRL.count(y) == 1) {
			for (int ind : grammar.unaryRL.at(y)) { // x -> y
				int x = grammar.unaryProductions[ind].first;
				long long e1 = make_fast_triple(i, x, j);
				tba.push_back(e1);
				if (trace) {
					record[e1].insert(e);
				}
			}
		}
		for (long long zk : adjacencyVector[j]) { // x -> yz
			int z = fast_pair_first(zk);
			int k = fast_pair_second(zk);
			if (grammar.binaryRL.count(make_fast_pair(y, z)) == 1) {
				for (int ind : grammar.binaryRL.at(make_fast_pair(y, z))) {
					int x = grammar.binaryProductions[ind].first;
					long long e1 = make_fast_triple(i, x, k);
					tba.push_back(e1);
					if (trace) {
						record[e1].insert(e);
						record[e1].insert(make_fast_triple(j, z, k));
					}
				}
			}
		}
		for (long long kz : counterAdjacencyVector[i]) { // x -> zy
			int k = fast_pair_first(kz);
			int z = fast_pair_second(kz);
			if (grammar.binaryRL.count(make_fast_pair(z, y)) == 1) {
				for (int ind : grammar.binaryRL.at(make_fast_pair(z, y))) {
					int x = grammar.binaryProductions[ind].first;
					long long e1 = make_fast_triple(k, x, j);
					tba.push_back(e1);
					if (trace) {
						record[e1].insert(make_fast_triple(k, z, i));
						record[e1].insert(e);
					}
				}
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

std::unordered_set<long long> Graph::getEdgeClosure(
	const Grammar &grammar,
	const std::vector<long long> &startSummaries,
	const std::unordered_map<long long, std::unordered_set<long long>> &record) const {
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
		int x = fast_triple_second(e);
		if (grammar.terminals.count(x) == 1) {
			closure.insert(e);
		} else {
			if (record.count(e) == 1) {
				for (long long e1 : record.at(e)) {
					if (vis.count(e1) == 0) {
						vis.insert(e1);
						w.push_back(e1);
					}
				}
			}
		}
	}
	return closure;
}

void Graph::clear() {
	numberOfVertices = 0;
	fastEdgeTest.clear();
	adjacencyVector.clear();
	counterAdjacencyVector.clear();
}
