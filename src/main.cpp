#include <vector>
#include <utility>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cassert>
#include <string>
#include <utility>
#include <tuple>
#include <algorithm>
#include <iterator>
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
	gm.terminals.insert(10);
	gm.nonterminals.insert(0);
	gm.nonterminals.insert(3);
	gm.emptyProductions.push_back(0);
	gm.unaryProductions.push_back(std::make_pair(0, 10));
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
	Graph gh(gm, 6);
	gh.addEdge(std::make_tuple(0, 1, 4));
	gh.addEdge(std::make_tuple(4, 10, 5));
	gh.addEdge(std::make_tuple(5, 2, 2));
	gh.addEdge(std::make_tuple(0, 1, 1));
	gh.addEdge(std::make_tuple(1, 2, 2));
	gh.addEdge(std::make_tuple(2, 1, 2));
	gh.addEdge(std::make_tuple(2, 2, 3));
	gh.addEdge(std::make_tuple(0, 1, 2));
	gh.runCFLReachability();
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if ((i == j) || (i == 0 && j == 2) || (i == 0 && j == 3) ||
			    (i == 2 && j == 3) || (i == 4 && j == 5)) {
				assert(gh.hasEdge(std::make_tuple(i, gm.startSymbol, j)));
			} else {
				assert(!gh.hasEdge(std::make_tuple(i, gm.startSymbol, j)));
			}
		}
	}
	auto rec1 = gh.getCFLReachabilityEdgeClosure(0, 2);
	assert(rec1.size() == 5);
	assert(rec1.count(std::make_tuple(0, 1, 1)) == 1);
	assert(rec1.count(std::make_tuple(1, 2, 2)) == 1);
	assert(rec1.count(std::make_tuple(0, 1, 4)) == 1);
	assert(rec1.count(std::make_tuple(4, 10, 5)) == 1);
	assert(rec1.count(std::make_tuple(5, 2, 2)) == 1);
	auto rec2 = gh.getCFLReachabilityEdgeClosure(2, 3);
	assert(rec2.size() == 2);
	assert(rec2.count(std::make_tuple(2, 1, 2)) == 1);
	assert(rec2.count(std::make_tuple(2, 2, 3)) == 1);
	auto rec3 = gh.getCFLReachabilityEdgeClosure(2, 2);
	assert(rec3.size() == 0);
}

void dumpVirtualMemoryPeak() {
        std::ifstream in("/proc/self/status");
        std::string line;
        while(getline(in, line)) {
                std::istringstream sin(line);
                std::string tag;
                sin >> tag;
                if (tag == "VmPeak:") {
                        std::cout << line << std::endl;
                        return;
                }
        }
}

int main(int argc, char *argv[]) {
	auto start = std::chrono::steady_clock::now();
	if (argc == 1) {
		test();
	} else {
		// read data
		const std::tuple<std::vector<Edge>, int, std::vector<Grammar>> data = parsePAGraph(argv[1]);
		std::cout << ">>> Completed Parsing" << std::endl;
		const auto &edges = std::get<0>(data);
		const int n = std::get<1>(data);
		const auto &grammars = std::get<2>(data);

		// construct graphs
		Graph gh1(grammars[0], n);
		gh1.fillEdges(edges);
		Graph gh2(grammars[1], n);
		gh2.fillEdges(edges);

		// first run of CFL reachability
		gh1.runCFLReachability();
		gh2.runCFLReachability();
		std::cout << ">>> Completed Preprocessing" << std::endl;

		// main query loop
		// int totalCFL1 = 0;
		// int totalCFL2 = 0;
		// int totalCFLBoolean = 0;
		int totalECFix = 0;
		for (int source = 0; source < n; source++) {
			if (source % 10 == 0) {
				std::cout << ">>> Query Progress: " << source << ',' << n - 1 << std::endl;
			}
			for (int sink = 0; sink < n; sink++) {
				Edge e1 = std::make_tuple(source, grammars[0].startSymbol, sink);
				Edge e2 = std::make_tuple(source, grammars[1].startSymbol, sink);
				bool reach1 = gh1.hasEdge(e1);
				bool reach2 = gh2.hasEdge(e2);
				// if (reach1) {
				// 	totalCFL1++;
				// }
				// if (reach2) {
				// 	totalCFL2++;
				// }
				if (reach1 && reach2) {
					// totalCFLBoolean++;
					auto c1 = gh1.getCFLReachabilityEdgeClosure(source, sink);
					auto c2 = gh2.getCFLReachabilityEdgeClosure(source, sink);
					std::set<Edge> es;
					std::set_intersection(c1.begin(), c1.end(), c2.begin(), c2.end(), std::inserter(es, es.begin()));
					while (true) {
						Graph gha(grammars[0], n);
						gha.fillEdges(es);
						gha.runCFLReachability();
						if (!(gha.hasEdge(e1))) {
							break;
						}
						auto ca = gha.getCFLReachabilityEdgeClosure(source, sink);
						Graph ghb(grammars[1], n);
						ghb.fillEdges(ca);
						ghb.runCFLReachability();
						if (!(ghb.hasEdge(e2))) {
							break;
						}
						auto cb = ghb.getCFLReachabilityEdgeClosure(source, sink);
						if (cb.size() == es.size()) {
							totalECFix++;
							break;
						} else {
							es = std::move(cb);
						}
					}
				}
			}
		}
		
		// std::cout << "totalCFL1: " << totalCFL1 << std::endl;
		// std::cout << "totalCFL2: " << totalCFL2 << std::endl;
		// std::cout << "totalCFLBoolean: " << totalCFLBoolean << std::endl;
		std::cout << "Total Pairs of ECFix: " << totalECFix << std::endl;
	}
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "Total Time (Seconds): " << elapsed_seconds.count() << std::endl;
	dumpVirtualMemoryPeak();
	return 0;
}
