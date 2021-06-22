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

void test();

void dumpVirtualMemoryPeak();

void printUsage(const char *name);

#define check_resource(str) do {\
	auto end = std::chrono::steady_clock::now();\
	std::chrono::duration<double> elapsed_seconds = end - start;\
	std::cout << "[--- begin resource check: " << str << " ---]" << std::endl;\
	std::cout << "Time (Seconds): " << elapsed_seconds.count() << std::endl;\
	dumpVirtualMemoryPeak();\
	std::cout << "[--- end resource check ---]" << std::endl;\
} while (0)

int main(int argc, char *argv[]) {
	auto start = std::chrono::steady_clock::now();
	if (argc == 2 && argv[1] == std::string("test")) {
		test();
	} else if (argc == 3 && argv[1] == std::string("pa-all-precise")) {
		// read data
		const std::tuple<std::map<std::string, int>, std::map<std::string, int>, std::vector<long long>, int, std::vector<Grammar>>
			data = parsePAGraph(argv[2]);
		std::cout << ">>> Completed Parsing" << std::endl;
		const auto &v_map = std::get<0>(data); // original vertex name -> number
		// const auto &l_map = std::get<1>(data);
		const auto &edges = std::get<2>(data);
		const int nv = std::get<3>(data);
		const auto &grammars = std::get<4>(data);

		std::map<int, std::string> counter_v_map; // number -> original vertex name
		for (auto &pr : v_map) {
			counter_v_map[pr.second] = pr.first;
		}

		// construct graphs
		Graph gh1(grammars[0], nv);
		gh1.fillEdges(edges);
		Graph gh2(grammars[1], nv);
		gh2.fillEdges(edges);

		// first run of CFL reachability
		gh1.runCFLReachability();
		gh2.runCFLReachability();
		std::cout << ">>> Completed Preprocessing" << std::endl;
		check_resource("preprocessing");

		// main query loop
		int totalL1 = 0;
		int totalL2 = 0;
		int totalBoolean = 0;
		int totalECFix = 0;
		for (int source = 0; source < nv; source++) {
			std::cout << ">>> Query Progress: " << source << ',' << nv - 1 << std::endl;
			for (int sink = 0; sink < nv; sink++) {
				long long e1 = make_fast_triple(source, grammars[0].startSymbol, sink);
				long long e2 = make_fast_triple(source, grammars[1].startSymbol, sink);
				bool reach1 = gh1.hasEdge(e1);
				bool reach2 = gh2.hasEdge(e2);
				if (reach1) {
					totalL1++;
				}
				if (reach2) {
					totalL2++;
				}
				if (reach1 && reach2) {
					totalBoolean++;
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
							totalECFix++;
							break;
						} else {
							es = std::move(cb);
						}
					}
				}
			}
		}
		
		std::cout << "totalL1: " << totalL1 << std::endl;
		std::cout << "totalL2: " << totalL2 << std::endl;
		std::cout << "totalBoolean: " << totalBoolean << std::endl;
		std::cout << "Total Pairs of ECFix: " << totalECFix << std::endl;
		check_resource("total");
	} else if (argc == 3 && argv[1] == std::string("pa-all")) {
		// read data
		const std::tuple<std::map<std::string, int>, std::map<std::string, int>, std::vector<long long>, int, std::vector<Grammar>>
			data = parsePAGraph(argv[2]);
		std::cout << ">>> Completed Parsing" << std::endl;
		const auto &v_map = std::get<0>(data); // original vertex name -> number
		// const auto &l_map = std::get<1>(data);
		const auto &edges = std::get<2>(data);
		const int nv = std::get<3>(data);
		const auto &grammars = std::get<4>(data);

		std::map<int, std::string> counter_v_map; // number -> original vertex name
		for (auto &pr : v_map) {
			counter_v_map[pr.second] = pr.first;
		}

		int totalECFix = 0;
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
							totalECFix++;
						}
					}
				}
				break;
			} else {
				es = std::move(ec2);
			}
		}
		std::cout << "Total Pairs of ECFix: " << totalECFix << std::endl;
		check_resource("total");
	} else if (argc == 6 && argv[1] == std::string("pa-ssss")) {
		// read data
		std::tuple<std::map<std::string, int>, std::map<std::string, int>, std::vector<long long>, int, std::vector<Grammar>>
			data = parsePAGraph(argv[2]);
		auto &v_map = std::get<0>(data); // original vertex name -> number
		auto &l_map = std::get<1>(data);
		const int nv = std::get<3>(data);
		const auto &grammars = std::get<4>(data);

		std::string edge_file = argv[3];
		int source = v_map[std::string(argv[4])];
		int sink = v_map[std::string(argv[5])];

		auto parsePALine = [](std::string line) {
			std::string::size_type p1, p2, p3, v1pos, v1len, v2pos, v2len, l1pos, l1len, l2pos, l2len;
			p1 = line.find("->");
			p2 = line.find('[');
			p3 = line.find(']');
			v1pos = 0;
			v1len = p1 - v1pos;
			v2pos = p1 + 2;
			v2len = p2 - v2pos;
			l1pos = p2 + 8;
			l1len = 2;
			l2pos = p2 + 12;
			l2len = p3 - 1 - l2pos;
			return std::make_pair(
					std::make_pair(line.substr(v1pos, v1len), line.substr(v2pos, v2len)),
					std::make_pair(line.substr(l1pos, l1len), line.substr(l2pos, l2len))
					);
		};

		std::ifstream in(edge_file); // file auto closed via destructor

		// read raw edges
		std::string line;
		std::vector<std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>>> rawEdges;
		while (getline(in, line)) {
			rawEdges.push_back(parsePALine(line));
		}

		std::unordered_set<long long> es;
		for (auto &ijtn : rawEdges) {
			es.insert(make_fast_triple(v_map[ijtn.first.first],
						l_map[ijtn.second.first + "--" + ijtn.second.second],
						v_map[ijtn.first.second]));
		}

		long long e1 = make_fast_triple(source, grammars[0].startSymbol, sink);
		long long e2 = make_fast_triple(source, grammars[1].startSymbol, sink);

		while (true) {
			Graph gha(grammars[0], nv);
			gha.fillEdges(es);
			gha.runCFLReachability();
			if (!(gha.hasEdge(e1))) {
				std::cout << "not reachable" << std::endl;
				return 0;
			}
			auto ca = gha.getCFLReachabilityEdgeClosure(source, sink);
			Graph ghb(grammars[1], nv);
			ghb.fillEdges(ca);
			ghb.runCFLReachability();
			if (!(ghb.hasEdge(e2))) {
				std::cout << "not reachable" << std::endl;
				return 0;
			}
			auto cb = ghb.getCFLReachabilityEdgeClosure(source, sink);
			if (cb.size() == es.size()) {
				std::cout << "possibly reachable" << std::endl;
				return 0;
			} else {
				es = std::move(cb);
			}
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
                        std::cout << line << std::endl;
                        return;
                }
        }
}

void printUsage(const char *name) {
	std::cerr << "Usage:" << std::endl
		<< '\t' << name << " test" << std::endl
		<< '\t' << name << " pa-all-precise" << " <graph-file-path>" << std::endl
		<< '\t' << name << " pa-all" << " <graph-file-path>" << std::endl
		<< '\t' << name << " pa-ssss" << " <graph-file-path> <edge-file> <source> <sink>" << std::endl
		<< '\t' << name << " bp" << " <graph-file-path>" << std::endl;
}
