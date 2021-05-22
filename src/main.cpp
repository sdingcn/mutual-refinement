#include <vector>
#include <utility>
#include <set>
#include <iostream>
#include <chrono>
#include <cassert>
#include "grammar/grammar.h"
#include "parser/parser.h"
#include "graph/graph.h"

void test() {
	/*
	 * D[0] -> D[0] D[0] | ([1] Dc[3] | epsilon
	 * Dc[3] -> D[0] )[2]
	 *
	 * 0 -> 0 0 | 1 3 | epsilon
	 * 3 -> 0 2
	 *
	 * 10 is a negligible terminal. We have 0 -> 10.
	 */
	Grammar gm;
	gm.terminals.insert(1);
	gm.terminals.insert(2);
	gm.nonterminals.insert(0);
	gm.nonterminals.insert(3);
	gm.emptyProductions.push_back(0);
	gm.binaryProductions.push_back(std::make_pair(0, std::make_pair(0, 0)));
	gm.binaryProductions.push_back(std::make_pair(0, std::make_pair(1, 3)));
	gm.binaryProductions.push_back(std::make_pair(3, std::make_pair(0, 2)));
	gm.startSymbol = 0;
	gm.fillInv(11);
	/*
	 *  1->(4)-10->(5)-2->
	 *  |                 \    -1->
	 *  |                  \  | /
	 * (0) --1--> (1) --2--> (2) --2--> (3)
	 *  |                     |
	 *   ----------1--------->
	 */
	Graph gh(6);
	gh.addEdge(0, 1, 4);
	gh.addEdge(4, 10, 5);
	gh.addEdge(5, 2, 2);
	gh.addEdge(0, 1, 1);
	gh.addEdge(1, 2, 2);
	gh.addEdge(2, 1, 2);
	gh.addEdge(2, 2, 3);
	gh.addEdge(0, 1, 2);
	gh.runCFLReachability(gm);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if ((i == j) || (i == 0 && j == 2) || (i == 0 && j == 3) ||
			    (i == 2 && j == 3) || (i == 4 && j == 5)) {
				assert(gh.hasEdge(i, gm.startSymbol, j));
			} else {
				assert(!gh.hasEdge(i, gm.startSymbol, j));
			}
		}
	}
	auto rec1 = gh.getCFLReachabilityEdgeClosure(0, 2, gm);
	assert(rec1.size() == 5);
	assert(rec1.count(make_edge(0, 1, 1)) == 1);
	assert(rec1.count(make_edge(1, 2, 2)) == 1);
	assert(rec1.count(make_edge(0, 1, 4)) == 1);
	assert(rec1.count(make_edge(4, 10, 5)) == 1);
	assert(rec1.count(make_edge(5, 2, 2)) == 1);
	auto rec2 = gh.getCFLReachabilityEdgeClosure(2, 3, gm);
	assert(rec2.size() == 2);
	assert(rec2.count(make_edge(2, 1, 2)) == 1);
	assert(rec2.count(make_edge(2, 2, 3)) == 1);
	auto rec3 = gh.getCFLReachabilityEdgeClosure(2, 2, gm);
	assert(rec3.size() == 0);
}

int main(int argc, char *argv[]) {
	auto start = std::chrono::steady_clock::now();
	if (argc == 1) {
		test();
	} else {
		// read data
		const std::pair<std::pair<std::vector<Edge>, std::pair<int, int>>, std::vector<Grammar>> data = parseFile(argv[1]);
		const std::vector<Edge> &edges = data.first.first;
		const std::vector<Grammar> &grammars = data.second;
		const int n = data.first.second.second + 1;

		// construct graphs
		Graph gh1(n);
		gh1.fillEdges(edges);
		Graph gh2 = gh1;

		// first run of CFL reachability
		gh1.runCFLReachability(grammars[0]);
		gh2.runCFLReachability(grammars[1]);

		// main query loop
		int totalCFL1 = 0;
		int totalCFL2 = 0;
		int totalCFLBoolean = 0;
		int totalEC = 0;
		int totalECFix = 0;
		for (int source = 0; source < n; source++) {
			std::cout << ">>> [main] Query Progress (Source Vertex): " << source << ',' << n - 1 << std::endl;
			for (int sink = 0; sink < n; sink++) {
				bool reach1 = gh1.hasEdge(source, grammars[0].startSymbol, sink);
				bool reach2 = gh2.hasEdge(source, grammars[1].startSymbol, sink);
				if (reach1) {
					totalCFL1++;
				}
				if (reach2) {
					totalCFL2++;
				}
				if (reach1 && reach2) {
					totalCFLBoolean++;
					
					// EC
					auto c1 = gh1.getCFLReachabilityEdgeClosure(source, sink, grammars[0]);
					auto c2 = gh2.getCFLReachabilityEdgeClosure(source, sink, grammars[1]);
					std::set<Edge> es;
					for (auto &e : c1) {
						if (c2.count(e) == 1) {
							es.insert(e);
						}
					}
					Graph gh(n);
					gh.fillEdges(es);
					if (gh.runPureReachability(source, sink)) {
						totalEC++;
					}

					// ECFix
					while (true) {
						Graph gh(n);
						gh.fillEdges(es);
						gh.runCFLReachability(grammars[0]);
						if (!(gh.hasEdge(source, grammars[0].startSymbol, sink))) {
							break;
						}
						auto c1 = gh.getCFLReachabilityEdgeClosure(source, sink, grammars[0]);
						gh = Graph(n);
						gh.fillEdges(c1);
						gh.runCFLReachability(grammars[1]);
						if (!(gh.hasEdge(source, grammars[1].startSymbol, sink))) {
							break;
						}
						auto c2 = gh.getCFLReachabilityEdgeClosure(source, sink, grammars[1]);
						if (c2.size() == es.size()) {
							totalECFix++;
							break;
						} else {
							es = c2;
						}
					}
				}
			}
		}
		
		std::cout << "totalCFL1: " << totalCFL1 << std::endl;
		std::cout << "totalCFL2: " << totalCFL2 << std::endl;
		std::cout << "totalCFLBoolean: " << totalCFLBoolean << std::endl;
		std::cout << "totalEC: " << totalEC << std::endl;
		std::cout << "totalECFix: " << totalECFix << std::endl;
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "Total Time (Seconds): " << elapsed_seconds.count() << std::endl;
	return 0;
}
