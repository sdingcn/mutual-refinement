#include "grammar/grammar.h"
#include "graph/graph.h"
#include "hasher/hasher.h"
#include "parser/parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <tuple>
#include <chrono>

int main(int argc, char *argv[]) {
	// type aliases
	using Edge = std::tuple<int, int, int>;
	using EdgeHasher = IntTripleHasher;
	// start time
	auto start = std::chrono::steady_clock::now();
	// main body
	if (argc == 2) {
		std::unordered_map<std::string, int> sym_map;
		std::vector<Grammar> grammars = extractDyck(argv[1], sym_map);
		int ng = grammars.size();
		std::unordered_map<std::string, int> node_map;
		auto tmp = parseGraph(argv[1], sym_map, node_map);
		int nv = tmp.first;
		std::unordered_set<Edge, EdgeHasher> edges = std::move(tmp.second);
		std::vector<Graph> graphs(ng);
#ifdef REFINE
		std::unordered_set<Edge, EdgeHasher>::size_type prev_size;
		do {
			prev_size = edges.size();
			for (int i = 0; i < ng; i++) {
				graphs[i].init(nv);
				graphs[i].addEdges(edges);
				std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
				auto summaries = graphs[i].runCFLReachability(grammars[i], true, record);
				edges = graphs[i].getEdgeClosure(grammars[i], summaries, record);
			}
		} while (edges.size() != prev_size);
#else
		for (int i = 0; i < ng; i++) {
			graphs[i].init(nv);
			graphs[i].addEdges(edges);
			std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
			graphs[i].runCFLReachability(grammars[i], false, record);
		}
#endif
		int ctr = 0;
		for (int s = 0; s < nv; s++) {
			for (int t = 0; t < nv; t++) {
				bool ok = true;
				for (int i = 0; i < ng; i++) {
					if (!(graphs[i].hasEdge(std::make_tuple(s, grammars[i].startSymbol, t)))) {
						ok = false;
						break;
					}
				}
				if (ok) {
					ctr++;
				}
			}
		}
		std::cout << ctr << std::endl;
	} else {
		std::cout << "Usage: " << argv[0] << " <graph-file-path>" << std::endl;
	}
	// resource check
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::ifstream in("/proc/self/status");
	std::string line;
	std::string vmpeak;
	while (getline(in, line)) {
		std::istringstream sin(line);
		std::string tag;
		sin >> tag;
		if (tag == "VmPeak:") {
			sin >> vmpeak;
			break;
		}
	}
	std::cout << "****** RESOURCE CONSUMPTION ******" << std::endl;
	std::cout << "Total Time (Seconds): " << elapsed_seconds.count() << std::endl;
	std::cout << "Peak Space (kB): " << vmpeak << std::endl; 
}
