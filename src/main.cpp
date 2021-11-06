#include <vector>
#include <utility>
#include <set>
#include <map>
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
	gm.addTerminal(1);
	gm.addTerminal(2);
	gm.addTerminal(10);
	gm.addNonterminal(0);
	gm.addNonterminal(3);
	gm.addEmptyProduction(0);
	gm.addUnaryProduction(0, 10);
	gm.addBinaryProduction(0, 0, 0);
	gm.addBinaryProduction(0, 1, 3);
	gm.addBinaryProduction(3, 0, 2);
	gm.addStartSymbol(0);
	gm.init(11);
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
			std::map<std::string, int>,
			std::map<std::vector<std::string>, int>,
			std::vector<long long>,
			std::vector<Grammar>
		> data = parsePAGraph(argv[1]);

		const std::map<std::string, int> &v_map              = std::get<0>(data);
#ifdef GRAPH
		const std::map<std::vector<std::string>, int> &l_map = std::get<1>(data);
		std::map<int, std::string> v_map_r;
		for (auto &p : v_map) {
			v_map_r[p.second] = p.first;
		}
		std::map<int, std::vector<std::string>> l_map_r;
		for (auto &p : l_map) {
			l_map_r[p.second] = p.first;
		}
#endif
		const std::vector<long long> &edges                  = std::get<2>(data);
		const std::vector<Grammar> &grammars                 = std::get<3>(data);
		const int &nv                                        = v_map.size();

#ifdef NAIVE
		Graph gh1(grammars[0], nv);
		gh1.fillEdges(edges);
		gh1.runCFLReachability();
		Graph gh2(grammars[1], nv);
		gh2.fillEdges(edges);
		gh2.runCFLReachability();
		Graph gh3(grammars[2], nv);
		gh3.fillEdges(edges);
		gh3.runCFLReachability();
		int ctr1 = 0, ctr2 = 0, ctr3 = 0, ctr = 0;
		for (int s = 0; s < nv; s++) {
			for (int t = 0; t < nv; t++) {
				bool r1 = gh1.hasEdge(make_fast_triple(s, grammars[0].startSymbol, t));
				bool r2 = gh2.hasEdge(make_fast_triple(s, grammars[1].startSymbol, t));
				bool r3 = gh3.hasEdge(make_fast_triple(s, grammars[2].startSymbol, t));
				if (r1) {
					ctr1++;
				}
				if (r2) {
					ctr2++;
				}
				if (r3) {
					ctr3++;
				}
				if (r1 && r2 && r3) {
					ctr++;
				}
			}
		}
		check_resource("naive combination");
		std::cout << "L1: " << ctr1 << std::endl;
		std::cout << "L2: " << ctr2 << std::endl;
		std::cout << "L3: " << ctr3 << std::endl;
		std::cout << "Comb: " << ctr << std::endl;
#endif

#ifdef REFINE
		std::unordered_set<long long> es(edges.begin(), edges.end());
		while (true) {
			Graph gh1(grammars[0], nv);
			gh1.fillEdges(es);
			gh1.runCFLReachability();
			auto es1 = gh1.getCFLReachabilityEdgeClosure(true);
			Graph gh2(grammars[1], nv);
			gh2.fillEdges(es1);
			gh2.runCFLReachability();
			auto es2 = gh2.getCFLReachabilityEdgeClosure(true);
			Graph gh3(grammars[2], nv);
			gh3.fillEdges(es2);
			gh3.runCFLReachability();
			auto es3 = gh3.getCFLReachabilityEdgeClosure(true);
			if (es3.size() == es.size()) {
				int ctr = 0;
				for (int s = 0; s < nv; s++) {
					for (int t = 0; t < nv; t++) {
						if (gh1.hasEdge(make_fast_triple(s, grammars[0].startSymbol, t)) &&
						gh2.hasEdge(make_fast_triple(s, grammars[1].startSymbol, t)) &&
						gh3.hasEdge(make_fast_triple(s, grammars[2].startSymbol, t))) {
							ctr++;
						}
					}
				}
				check_resource("refined combination");
				std::cout << "Comb:" << ctr << std::endl;
				break;
			} else {
				es = std::move(es3);
			}
		}
#endif

#ifdef GRAPH
		std::unordered_set<long long> es(edges.begin(), edges.end());
		while (true) {
			Graph gh1(grammars[0], nv);
			gh1.fillEdges(es);
			gh1.runCFLReachability();
			auto es1 = gh1.getCFLReachabilityEdgeClosure(true);
			Graph gh2(grammars[1], nv);
			gh2.fillEdges(es1);
			gh2.runCFLReachability();
			auto es2 = gh2.getCFLReachabilityEdgeClosure(true);
			Graph gh3(grammars[2], nv);
			gh3.fillEdges(es2);
			gh3.runCFLReachability();
			auto es3 = gh3.getCFLReachabilityEdgeClosure(true);
			if (es3.size() == es.size()) {
				std::ofstream out("simplified.dot");
				// 100->200[label="cp--10"]
				for (long long e : es) {
					int i = fast_triple_first(e);
					int x = fast_triple_second(e);
					int j = fast_triple_third(e);
					out << v_map_r[i] << "->"
						<< v_map_r[j] << "[label=\""
						<< l_map_r[x][0] << "--"
						<< l_map_r[x][1] << "\"]\n";
				}
				check_resource("graph simplification");
				break;
			} else {
				es = std::move(es3);
			}
		}
#endif
	} else {
		test();
		check_resource("test");
		printUsage(argv[0]);
	}
}
