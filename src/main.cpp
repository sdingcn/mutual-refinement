#include <vector>
#include <utility>
#include <set>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cassert>
#include <cstdlib>
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
	gh.addEdge(make_fast_triple(0, 1, 4));
	gh.addEdge(make_fast_triple(4, 10, 5));
	gh.addEdge(make_fast_triple(5, 2, 2));
	gh.addEdge(make_fast_triple(0, 1, 1));
	gh.addEdge(make_fast_triple(1, 2, 2));
	gh.addEdge(make_fast_triple(2, 1, 2));
	gh.addEdge(make_fast_triple(2, 2, 3));
	gh.addEdge(make_fast_triple(0, 1, 2));
	gh.runCFLReachability();
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if ((i == j) || (i == 0 && j == 2) || (i == 0 && j == 3) ||
			    (i == 2 && j == 3) || (i == 4 && j == 5)) {
				assert(gh.hasEdge(make_fast_triple(i, gm.startSymbol, j)));
			} else {
				assert(!gh.hasEdge(make_fast_triple(i, gm.startSymbol, j)));
			}
		}
	}
	{
		auto rec1 = gh.getCFLReachabilityEdgeClosure(false, 0, 2);
		assert(rec1.size() == 5);
		assert(rec1.count(make_fast_triple(0, 1, 1)) == 1);
		assert(rec1.count(make_fast_triple(1, 2, 2)) == 1);
		assert(rec1.count(make_fast_triple(0, 1, 4)) == 1);
		assert(rec1.count(make_fast_triple(4, 10, 5)) == 1);
		assert(rec1.count(make_fast_triple(5, 2, 2)) == 1);
	}
	{
		auto rec2 = gh.getCFLReachabilityEdgeClosure(false, 2, 3);
		assert(rec2.size() == 2);
		assert(rec2.count(make_fast_triple(2, 1, 2)) == 1);
		assert(rec2.count(make_fast_triple(2, 2, 3)) == 1);
	}
	{
		auto rec3 = gh.getCFLReachabilityEdgeClosure(false, 2, 2);
		assert(rec3.size() == 0);
	}
}

void dumpVirtualMemoryPeak() {
        std::ifstream in("/proc/self/status");
        std::string line;
        while(getline(in, line)) {
                std::istringstream sin(line);
                std::string tag;
                sin >> tag;
                if (tag == "VmPeak:") {
                        std::cout << "    " << line << std::endl;
                        return;
                }
        }
}

void printUsage(const char *name) {
	std::cerr << "Passed all tests." << std::endl;
	std::cerr << "Usage: " << name << " <graph-file-path>" << std::endl;
}

#define check_resource(str) do {\
	auto end = std::chrono::steady_clock::now();\
	std::chrono::duration<double> elapsed_seconds = end - start;\
	std::cout << '{' << std::endl;\
	std::cout << "    resource check: " << str << std::endl;\
	std::cout << "    Time (Seconds): " << elapsed_seconds.count() << std::endl;\
	dumpVirtualMemoryPeak();\
	std::cout << '}' << std::endl;\
} while (0)

int main(int argc, char *argv[]) {
	auto start = std::chrono::steady_clock::now();
	if (argc == 2) {
		// parse data
		const std::tuple<
			std::unordered_map<std::string, int>,
			std::unordered_map<std::string, int>,
			std::vector<long long>,
			std::vector<Grammar>
		> data = parsePAGraph(argv[1]);

		// obtain references to the original data
		const int &nv                           = std::get<0>(data).size();
		const std::vector<long long> &edges     = std::get<2>(data);
		const std::vector<Grammar> &grammars    = std::get<3>(data);

		Graph gh1(grammars[0], nv);
		gh1.fillEdges(edges);
		gh1.runCFLReachability();
		int ctr1 = 0;
		for (int s = 0; s < nv; s++) {
			for (int t = 0; t < nv; t++) {
				if (gh1.hasEdge(make_fast_triple(s, grammars[0].startSymbol, t))) {
					ctr1++;
				}
			}
		}
		Graph gh2(grammars[1], nv);
		gh2.fillEdges(edges);
		gh2.runCFLReachability();
		int ctr2 = 0;
		for (int s = 0; s < nv; s++) {
			for (int t = 0; t < nv; t++) {
				if (gh2.hasEdge(make_fast_triple(s, grammars[1].startSymbol, t))) {
					ctr2++;
				}
			}
		}
		check_resource("CFL1 and CFL2");
		std::cout << "CFL1 = " << ctr1 << ", CFL2 = " << ctr2 << std::endl;
	} else {
		test();
		check_resource("TEST");
		printUsage(argv[0]);
	}
}
