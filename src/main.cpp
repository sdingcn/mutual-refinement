#include <vector>
#include <utility>
#include <set>
#include <map>
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
#ifdef INTEGRATION
#include "../CFLReach.h"
#endif

void test();

void dumpVirtualMemoryPeak();

void printUsage(const char *name);

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
	if (argc == 2 && argv[1] == std::string("test")) {
		test();
	} else if (argc == 4 && argv[1] == std::string("pa")) {
		// parse data
		const std::tuple<std::map<std::string, int>, std::map<std::string, int>, std::vector<long long>, int, std::vector<Grammar>>
			data = parsePAGraph(argv[3]);
		check_resource("Parsing");

		// obtain references to the original data
		const auto &edges = std::get<2>(data);
		const int &nv = std::get<3>(data);
		const auto &grammars = std::get<4>(data);

		// CFL1
		if (argv[2] == std::string("cfl1")) {
			Graph gh1(grammars[0], nv);
			gh1.fillEdges(edges);
			gh1.runCFLReachability();
			int totalCFL1 = 0;
			for (int source = 0; source < nv; source++) {
				for (int sink = 0; sink < nv; sink++) {
					if (gh1.hasEdge(make_fast_triple(source, grammars[0].startSymbol, sink))) {
						totalCFL1++;
					}
				}
			}
			std::cout << "CFL1: " << totalCFL1 << std::endl;
			check_resource("CFL1");
		} else if (argv[2] == std::string("cfl2")) {
			Graph gh2(grammars[1], nv);
			gh2.fillEdges(edges);
			gh2.runCFLReachability();
			int totalCFL2 = 0;
			for (int source = 0; source < nv; source++) {
				for (int sink = 0; sink < nv; sink++) {
					if (gh2.hasEdge(make_fast_triple(source, grammars[1].startSymbol, sink))) {
						totalCFL2++;
					}
				}
			}
			std::cout << "CFL2: " << totalCFL2 << std::endl;
			check_resource("CFL2");
		} else if (argv[2] == std::string("cflbool")){
			Graph gh1(grammars[0], nv);
			gh1.fillEdges(edges);
			gh1.runCFLReachability();
			Graph gh2(grammars[1], nv);
			gh2.fillEdges(edges);
			gh2.runCFLReachability();
			int totalBoolean = 0;
			for (int source = 0; source < nv; source++) {
				for (int sink = 0; sink < nv; sink++) {
					if (gh1.hasEdge(make_fast_triple(source, grammars[0].startSymbol, sink))
					 && gh2.hasEdge(make_fast_triple(source, grammars[1].startSymbol, sink))) {
						totalBoolean++;
					}
				}
			}
			std::cout << "CFL Boolean: " << totalBoolean << std::endl;
			check_resource("CFL Boolean");
		} else if (argv[2] == std::string("cflapmr")) {
			int totalAPMR = 0;
			std::unordered_set<long long> es;
			for (long long e : edges) {
				es.insert(e);
			}
			while (true) {
				Graph gh1(grammars[0], nv);
				gh1.fillEdges(es);
				gh1.runCFLReachability();
				auto ec1 = gh1.getCFLReachabilityEdgeClosureAll();
				Graph gh2(grammars[1], nv);
				gh2.fillEdges(ec1);
				gh2.runCFLReachability();
				auto ec2 = gh2.getCFLReachabilityEdgeClosureAll();
				if (ec2.size() == es.size()) {
					for (int source = 0; source < nv; source++) {
						for (int sink = 0; sink < nv; sink++) {
							long long e1 = make_fast_triple(source, grammars[0].startSymbol, sink);
							long long e2 = make_fast_triple(source, grammars[1].startSymbol, sink);
							if (gh1.hasEdge(e1) && gh2.hasEdge(e2)) {
								totalAPMR++;
							}
						}
					}
					break;
				} else {
					es = std::move(ec2);
				}
			}
			std::cout << "CFL All Pairs Mutual Refinement: " << totalAPMR << std::endl;
			check_resource("CFL All Pairs Mutual Refinement");
		} else if (argv[2] == std::string("cflspmr")) {
			Graph gh1(grammars[0], nv);
			gh1.fillEdges(edges);
			Graph gh2(grammars[1], nv);
			gh2.fillEdges(edges);

			gh1.runCFLReachability();
			gh2.runCFLReachability();
			check_resource("CFLSPMR Preprocessing");

			int totalSPMR = 0;
			for (int source = 0; source < nv; source++) {
				for (int sink = 0; sink < nv; sink++) {
					long long e1 = make_fast_triple(source, grammars[0].startSymbol, sink);
					long long e2 = make_fast_triple(source, grammars[1].startSymbol, sink);
					if (gh1.hasEdge(e1) && gh2.hasEdge(e2)) {
						std::unordered_set<long long> es;
						{
							auto c1 = gh1.getCFLReachabilityEdgeClosure(source, sink);
							auto c2 = gh2.getCFLReachabilityEdgeClosure(source, sink);
							for (long long e : c1) {
								if (c2.count(e) == 1) {
									es.insert(e);
								}
							}
						}
						while (true) {
							Graph gha(grammars[0], nv);
							gha.fillEdges(es);
							gha.runCFLReachability();
							if (!(gha.hasEdge(e1))) {
								break;
							}
							auto ca = gha.getCFLReachabilityEdgeClosure(source, sink);
							Graph ghb(grammars[1], nv);
							ghb.fillEdges(ca);
							ghb.runCFLReachability();
							if (!(ghb.hasEdge(e2))) {
								break;
							}
							auto cb = ghb.getCFLReachabilityEdgeClosure(source, sink);
							if (cb.size() == es.size()) {
								totalSPMR++;
								break;
							} else {
								es = std::move(cb);
							}
						}
					}
				}
			}
			std::cout << "CFL Single Pair Mutual Refinement: " << totalSPMR << std::endl;
			check_resource("CFL Single Pair Mutual Refinement");
#ifdef INTEGRATION
		} else if (argv[2] == std::string("lcl")) {
			// prepare converters
			const auto &v_map = std::get<0>(data); // original vertex name -> number
			std::map<int, std::string> v_map_r;
			for (auto &pr : v_map) {
				v_map_r[pr.second] = pr.first;
			}
			const auto &l_map = std::get<1>(data); // original edge label -> number
			std::map<int, std::string> l_map_r;
			for (auto &pr : l_map) {
				l_map_r[pr.second] = pr.first;
			}

			// begin lcl computation
			std::stringstream lcl_buffer;
			for (long long e : edges) {
				int v1 = fast_triple_first(e);
				int l = fast_triple_second(e);
				int v2 = fast_triple_third(e);
				lcl_buffer << v_map_r[v1] << "->" << v_map_r[v2] << "[label=\"" << l_map_r[l] << "\"]\n";
			}
			SimpleDotParser lcl_parser;
			std::unordered_map<std::string, unsigned> lcl_v_map;
			std::unordered_map<unsigned, std::string> lcl_v_map_r;
			unsigned lcl_nv = lcl_parser.BuildNodeMap(lcl_buffer, lcl_v_map, lcl_v_map_r);
			std::vector<std::vector<std::unordered_set<std::string>>>
				lcl_output(lcl_nv, std::vector<std::unordered_set<std::string>>(lcl_nv));
			arrayversion("1", "1", "1", lcl_v_map, lcl_v_map_r, lcl_nv, lcl_buffer, lcl_output);
			check_resource("LCL");
		} else if (argv[2] == std::string("lclbool")) {
			check_resource("LCL Boolean");
		} else if (argv[2] == std::string("lclapmr")) {
			check_resource("LCL All Pairs Mutual Refinement");
		} else if (argv[2] == std::string("lclspmr")) {
			check_resource("LCL Single Pair Mutual Refinement");
#endif
		} else {
			printUsage(argv[0]);
		}
	} else if (argc == 3 && argv[1] == std::string("bp")) {
		std::tuple<std::vector<long long>, int, std::pair<int, int>, std::vector<Grammar>> data = parseBPGraph(argv[2]);
		const auto &edges = std::get<0>(data);
		const int nv = std::get<1>(data);
		const int source = std::get<2>(data).first;
		const int sink = std::get<2>(data).second;
		const auto &grammars = std::get<3>(data);
		const int ng = grammars.size();

		// TODO: change to pure reachability closure, and only proceed if purely reachable
		std::unordered_set<long long> es(edges.begin(), edges.end());
		int ite = 0;
		while (true) {
			std::cerr << "ITERATION " << (++ite) << std::endl;
			auto es_size = es.size();
			for (int i = 0; i < ng; i++) {
				std::cerr << "GRAMMAR " << i << std::endl;
				Graph gh(grammars[i], nv);
				gh.fillEdges(es);
				gh.runCFLReachability();
				std::cerr << "DONE CFLReach" << std::endl;
				if (!gh.hasEdge(make_fast_triple(source, grammars[i].startSymbol, sink))) {
					std::cout << "Definitely Unreachable" << std::endl;
					goto END;
				}
				es = gh.getCFLReachabilityEdgeClosure(source, sink);
				std::cerr << "DONE EdgeClosure" << std::endl;
			}
			if (es.size() == es_size) {
				break;
			}
		}
		std::cout << "Possibly Reachable" << std::endl;
END:;
	} else {
		printUsage(argv[0]);
	}
}

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
		auto rec1 = gh.getCFLReachabilityEdgeClosure(0, 2);
		assert(rec1.size() == 5);
		assert(rec1.count(make_fast_triple(0, 1, 1)) == 1);
		assert(rec1.count(make_fast_triple(1, 2, 2)) == 1);
		assert(rec1.count(make_fast_triple(0, 1, 4)) == 1);
		assert(rec1.count(make_fast_triple(4, 10, 5)) == 1);
		assert(rec1.count(make_fast_triple(5, 2, 2)) == 1);
	}
	{
		auto rec2 = gh.getCFLReachabilityEdgeClosure(2, 3);
		assert(rec2.size() == 2);
		assert(rec2.count(make_fast_triple(2, 1, 2)) == 1);
		assert(rec2.count(make_fast_triple(2, 2, 3)) == 1);
	}
	{
		auto rec3 = gh.getCFLReachabilityEdgeClosure(2, 2);
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
	std::cerr << "Usage:" << std::endl
		<< '\t' << name << " test" << std::endl
		<< '\t' << name <<
			" pa <cfl1|cfl2|cflbool|cflapmr|cflspmr"
#ifdef INTEGRATION
			"|lcl|lclbool|lclapmr|lclspmr"
#endif
			"> <graph-file-path>" << std::endl
		<< '\t' << name << " bp <graph-file-path>" << std::endl;
}
