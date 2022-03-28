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

int main(int argc, char *argv[]) {
	// resource check init
	auto start = std::chrono::steady_clock::now();
	// main body
	if (argc == 2) {
#ifdef NAIVE
		std::unordered_map<std::string, int> sym_map;
		std::vector<Grammar> grammars = extractDyck(argv[1], sym_map);
		int ng = grammars.size();
		std::unordered_map<std::string, int> node_map;
		auto p = parseGraph(argv[1], sym_map, node_map);
		int nv = p.first;
		std::unordered_set<long long> edges = std::move(p.second);
		std::vector<Graph> graphs(ng);
		for (int i = 0; i < ng; i++) {
			graphs[i].init(nv);
			graphs[i].addEdges(edges);
			std::unordered_map<long long, std::unordered_set<long long>> record;
			graphs[i].runCFLReachability(grammars[i], false, record);
		}
		int ctr = 0;
		for (int s = 0; s < nv; s++) {
			for (int t = 0; t < nv; t++) {
				bool ok = true;
				for (int i = 0; i < ng; i++) {
					ok = ok && graphs[i].hasEdge(make_fast_triple(s, grammars[i].startSymbol, t));
				}
				if (ok) {
					ctr++;
				}
			}
		}
		std::cout << "naive: " << ctr << std::endl;
#endif
#ifdef REFINE
		std::unordered_map<std::string, int> sym_map;
		std::vector<Grammar> grammars = extractDyck(argv[1], sym_map);
		int ng = grammars.size();
		std::unordered_map<std::string, int> node_map;
		auto p = parseGraph(argv[1], sym_map, node_map);
		int nv = p.first;
		std::unordered_set<long long> edges = std::move(p.second);
		while (true) {
			std::unordered_set<long long>::size_type prev_size = edges.size();
			std::vector<Graph> graphs(ng);
			for (int i = 0; i < ng; i++) {
				graphs[i].init(nv);
				graphs[i].addEdges(edges);
				std::unordered_map<long long, std::unordered_set<long long>> record;
				auto summaries = graphs[i].runCFLReachability(grammars[i], true, record);
				edges = graphs[i].getEdgeClosure(grammars[i], summaries, record);
			}
			if (edges.size() == prev_size) {
				int ctr = 0;
				for (int s = 0; s < nv; s++) {
					for (int t = 0; t < nv; t++) {
						bool ok = true;
						for (int i = 0; i < ng; i++) {
							ok = ok && graphs[i].hasEdge(make_fast_triple(s, grammars[i].startSymbol, t));
						}
						if (ok) {
							ctr++;
						}
					}
				}
				std::cout << "refine: " << ctr << std::endl;
				break;
			}
		}
#endif
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
	std::cout << "****** RESOURCE CHECK ******" << std::endl;
	std::cout << "Total Time (Seconds): " << elapsed_seconds.count() << std::endl;
	std::cout << "Peak Space (kB): " << vmpeak << std::endl; 
}
