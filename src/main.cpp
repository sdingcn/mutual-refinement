#include <vector>
#include <unordered_set>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cassert>
#include <string>
#include <utility>
#include <tuple>
#include "grammar/grammar.h"
#include "parser/parser.h"
#include "graph/graph.h"

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
	if (argc == 2) {
		// start timer
		auto start = std::chrono::steady_clock::now();

		// parse data
		const std::tuple<
			std::map<std::string, int>,
			std::map<std::vector<std::string>, int>,
			std::unordered_set<long long>,
			std::vector<Grammar>
		> data = parsePAGraph(argv[1]);

		// prepare data
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
		const std::unordered_set<long long> &edges           = std::get<2>(data);
		const std::vector<Grammar> &grammars                 = std::get<3>(data);
		const int &nv                                        = v_map.size();

#ifdef NAIVE
		Graph gh1(grammars[0], nv, edges);
		gh1.runCFLReachability();
		Graph gh2(grammars[1], nv, edges);
		gh2.runCFLReachability();
		Graph gh3(grammars[2], nv, edges);
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
			Graph gh1(grammars[0], nv, es);
			gh1.runCFLReachability();
			auto es1 = gh1.getCFLReachabilityEdgeClosure();
			Graph gh2(grammars[1], nv, es1);
			gh2.runCFLReachability();
			auto es2 = gh2.getCFLReachabilityEdgeClosure();
			Graph gh3(grammars[2], nv, es2);
			gh3.runCFLReachability();
			auto es3 = gh3.getCFLReachabilityEdgeClosure();
			if (es3.size() == es.size()) {
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
				check_resource("refined combination");
				std::cout << "L1: " << ctr1 << std::endl;
				std::cout << "L2: " << ctr2 << std::endl;
				std::cout << "L3: " << ctr3 << std::endl;
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
			Graph gh1(grammars[0], nv, es);
			gh1.runCFLReachability();
			auto es1 = gh1.getCFLReachabilityEdgeClosure();
			Graph gh2(grammars[1], nv, es1);
			gh2.runCFLReachability();
			auto es2 = gh2.getCFLReachabilityEdgeClosure();
			Graph gh3(grammars[2], nv, es2);
			gh3.runCFLReachability();
			auto es3 = gh3.getCFLReachabilityEdgeClosure();
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
		printUsage(argv[0]);
	}
}
