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
#ifdef INTEGRATION
#include "../CFLReach.h"
#include "../bitmap.h"
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
	} else if (argc == 5 && argv[1] == std::string("pa")) {
		// parse data
		const std::tuple<std::map<std::string, int>, std::map<std::string, int>, std::vector<long long>, int, std::vector<Grammar>>
			data = parsePAGraph(argv[4]);

		// obtain references to the original data
		const std::map<std::string, int> &v_map = std::get<0>(data); // original vertex name -> number
		const std::map<std::string, int> &l_map = std::get<1>(data); // original edge label -> number
		const std::vector<long long> &edges     = std::get<2>(data);
		const int &nv                           = std::get<3>(data);
		const std::vector<Grammar> &grammars    = std::get<4>(data);

		// converters
		std::map<int, std::string> v_map_r_core;
		for (auto &pr : v_map) {
			v_map_r_core[pr.second] = pr.first;
		}
		std::map<int, std::string> l_map_r_core;
		for (auto &pr : l_map) {
			l_map_r_core[pr.second] = pr.first;
		}
		const std::map<int, std::string> &v_map_r = v_map_r_core;
		const std::map<int, std::string> &l_map_r = l_map_r_core;

		// type aliases
		using PairSet = std::unordered_set<long long>;
		using EdgeSet = std::unordered_set<long long>;

		// workers
		auto cfl_all = [&v_map, &v_map_r, &l_map, &l_map_r, &nv]
			(const Grammar &gr, const EdgeSet &es, bool need_ps, bool need_es) -> std::pair<PairSet, EdgeSet> {
			Graph gh(gr, nv);
			gh.fillEdges(es);
			gh.runCFLReachability();
			auto ret_ps = PairSet();
			if (need_ps) {
				for (int s = 0; s < nv; s++) {
					for (int t = 0; t < nv; t++) {
						if (gh.hasEdge(make_fast_triple(s, gr.startSymbol, t))) {
							ret_ps.insert(make_fast_pair(s, t));
						}
					}
				}
			}
			auto ret_es = EdgeSet();
			if (need_es) {
				ret_es = gh.getCFLReachabilityEdgeClosureAll();
			}
			return std::make_pair(ret_ps, ret_es);
		};
#ifdef INTEGRATION
		auto lcl_all = [&v_map, &v_map_r, &l_map, &l_map_r, &nv]
			(const EdgeSet &es, bool need_ps, bool need_es) -> std::pair<PairSet, EdgeSet> {
			std::stringstream buffer;

			// convert my edge set to the raw edge set
			for (long long e : es) {
				int v1 = fast_triple_first(e);
				int l = fast_triple_second(e);
				int v2 = fast_triple_third(e);
				buffer << v_map_r.at(v1) << "->" << v_map_r.at(v2) << "[label=\"" << l_map_r.at(l) << "\"]\n";
			}

			// parse the raw edge set
			SimpleDotParser dotparser;
			std::unordered_map<std::string, unsigned> NodeID;
			std::unordered_map<unsigned, std::string> NodeID_R;
			unsigned NodeNum = dotparser.BuildNodeMap(buffer, NodeID, NodeID_R);

			// do the LCL computation
			std::vector<std::vector<std::unordered_map<LinEdgeTy, std::unordered_set<LinEdgeTy>>>> trace_L(NodeNum);
			std::vector<std::vector<std::unordered_map<LinEdgeTy, std::unordered_set<LinEdgeTy>>>> trace_R(NodeNum);
			CFLMatrixLin cm1(NodeNum);
			bitmap_obstack_initialize(NULL);
			bitmap* S[NodeNum]; // VLA
			std::vector<std::vector<std::unordered_set<std::string>>> orig_graph(NodeNum);
			long **observed  = (long **)std::malloc(sizeof(long *)*NodeNum);
			long **goodq2 = (long **)std::malloc(sizeof(long *)*NodeNum);
			LCLReach(NodeID, NodeID_R, NodeNum, buffer, trace_L, trace_R, cm1, S, orig_graph, observed, goodq2);
			
			unsigned q2_mLin = q2Lin*numLinEdgeTy + mLin;

			auto ret_ps = PairSet();
			if (need_ps) {
				for (unsigned ii = 0; ii < NodeNum; ii++) {
					for (unsigned jj = 0; jj < NodeNum; jj++) {
						unsigned NodeS = ii;
						unsigned NodeT = jj;
						if (NodeS == NodeT) {
							ret_ps.insert(make_fast_pair(NodeS, NodeT));
						} else {
							if (TestItemInSet(observed[NodeS], NodeT) &&
								bitmap_bit_p(S[NodeS][NodeT], q2_mLin) &&
								TestItemInSet(goodq2[NodeS], NodeT)) {
								ret_ps.insert(make_fast_pair(NodeS, NodeT));
							}
						}
					}
				}
			}

			auto ret_es = EdgeSet();
			if (need_es) {
				std::vector<std::vector<std::unordered_set<std::string>>>
					output(NodeNum, std::vector<std::unordered_set<std::string>>(NodeNum));
				std::vector<std::vector<std::unordered_set<LinEdgeTy>>>
					visited(NodeNum, std::vector<std::unordered_set<LinEdgeTy>>(NodeNum));
				for (unsigned ii = 0; ii < NodeNum; ii++) {
					for (unsigned jj = 0; jj < NodeNum; jj++) {
						unsigned NodeS = ii;
						unsigned NodeT = jj;
						if (TestItemInSet(observed[NodeS], NodeT) &&
							bitmap_bit_p(S[NodeS][NodeT], q2_mLin) &&
							TestItemInSet(goodq2[NodeS], NodeT)) {
							dfs(NodeS, NodeT, q2_mLin, trace_L, trace_R, cm1, visited, S, observed, orig_graph, NodeID_R, output);
						}
					}
				}
				for (unsigned i = 0; i < NodeNum; i++) {
					for (unsigned j = 0; j < NodeNum; j++) {
						if (output[i][j].size() > 0) {
							for (auto x : output[i][j]) {
								ret_es.insert(
									make_fast_triple(v_map.at(NodeID_R[i]), l_map.at(x), v_map.at(NodeID_R[j]))
								);
							}
						}
					}
				}
			}
			std::free(observed);
			std::free(goodq2);
			return std::make_pair(ret_ps, ret_es);
		};
#endif

#ifdef INTEGRATION
		if (argv[2] == std::string("apmr") && argv[3] == std::string("c1l")) {
			std::unordered_set<long long> es;
			for (long long e : edges) {
				es.insert(e);
			}
			int round = 0;
			while (true) {
				std::cout << ">>> Round " << (++round) << std::endl;
				auto r1 = cfl_all(grammars[0], es, true, true);
				auto es1 = r1.second;
				auto r2 = lcl_all(es1, true, true);
				auto es2 = r2.second;
				if (es2.size() == es.size()) {
					auto ps1 = r1.first;
					auto ps2 = r2.first;
					PairSet ps;
					for (auto p : ps1) {
						if (ps2.count(p) > 0) {
							ps.insert(p);
						}
					}
					std::cout << ">>> APMR C1L: " << ps.size() << std::endl;
					break;
				} else {
					es = std::move(es2);
				}
			}
		} else
#endif
		{
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
			" pa <bool|apmr|spmr> <c1c2"
#ifdef INTEGRATION
			"|c1l|c2l|c1c2l"
#endif
			"> <graph-file-path>" << std::endl
		<< '\t' << name << " bp <graph-file-path>" << std::endl;
}
