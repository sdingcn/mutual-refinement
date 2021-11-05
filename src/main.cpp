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

		// obtain references to the original data
		const int &nv                           = std::get<0>(data).size();
		const std::vector<long long> &edges     = std::get<2>(data);
		const std::vector<Grammar> &grammars    = std::get<3>(data);

#if 1
// naive
{
		Graph gh1(grammars[0], nv);
		gh1.fillEdges(edges);
		gh1.runCFLReachability();
		Graph gh2(grammars[1], nv);
		gh2.fillEdges(edges);
		gh2.runCFLReachability();
		Graph gh3(grammars[2], nv);
		gh3.fillEdges(edges);
		gh3.runCFLReachability();
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
		std::cout << "Naive: " << ctr << std::endl;
}
#endif

#if 1
// apmr
{
		std::unordered_set<long long> es(edges.begin(), edges.end());
#ifdef VERBOSE
		int iter = 0;
#endif
		while (true) {
#ifdef VERBOSE
			std::cerr << "APMR Iteration: " << (++iter) << ", Edge Set Size: " << es.size() << std::endl;
#endif
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
				std::cout << "APMR:" << ctr << std::endl;
				break;
			} else {
				es = std::move(es3);
			}
		}
}
#endif

#if 0
// spmr
{
		// pre run
		Graph gh1(grammars[0], nv);
		gh1.fillEdges(edges);
		gh1.runCFLReachability();
		Graph gh2(grammars[1], nv);
		gh2.fillEdges(edges);
		gh2.runCFLReachability();
		Graph gh3(grammars[2], nv);
		gh3.fillEdges(edges);
		gh3.runCFLReachability();
		int ctr = 0;
		for (int s = 0; s < nv; s++) {
			for (int t = 0; t < nv; t++) {
#ifdef VERBOSE
				std::cerr << "s = " << s << ", t = " << t << std::endl;
#endif
				if (gh1.hasEdge(make_fast_triple(s, grammars[0].startSymbol, t)) &&
				gh2.hasEdge(make_fast_triple(s, grammars[1].startSymbol, t)) &&
				gh3.hasEdge(make_fast_triple(s, grammars[2].startSymbol, t))) {
					std::unordered_set<long long> es;
					{
						auto c1 = gh1.getCFLReachabilityEdgeClosure(false, s, t);
						auto c2 = gh2.getCFLReachabilityEdgeClosure(false, s, t);
						auto c3 = gh3.getCFLReachabilityEdgeClosure(false, s, t);
						for (long long e : c1) {
							if (c2.count(e) == 1 && c3.count(e) == 1) {
								es.insert(e);
							}
						}
					}
					while (true) {
						Graph gha(grammars[0], nv);
						gha.fillEdges(es);
						gha.runCFLReachability();
						if (!gha.hasEdge(make_fast_triple(s, grammars[0].startSymbol, t))) {
							break;
						}
						auto ca = gha.getCFLReachabilityEdgeClosure(false, s, t);
						Graph ghb(grammars[1], nv);
						ghb.fillEdges(ca);
						ghb.runCFLReachability();
						if (!ghb.hasEdge(make_fast_triple(s, grammars[1].startSymbol, t))) {
							break;
						}
						auto cb = ghb.getCFLReachabilityEdgeClosure(false, s, t);
						Graph ghc(grammars[2], nv);
						ghc.fillEdges(cb);
						ghc.runCFLReachability();
						if (!ghc.hasEdge(make_fast_triple(s, grammars[2].startSymbol, t))) {
							break;
						}
						auto cc = ghc.getCFLReachabilityEdgeClosure(false, s, t);
						if (cc.size() == es.size()) {
							ctr++;
							break;
						} else {
							es = std::move(cc);
						}
					}
				}
			}
		}
		std::cout << "SPMR:" << ctr << std::endl;
}
#endif
	} else {
		test();
		check_resource("TEST");
		printUsage(argv[0]);
	}
}
