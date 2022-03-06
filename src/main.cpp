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

void check_resource(void (*f)()) {
	auto start = std::chrono::steady_clock::now();
	f();
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

void run() {
	int ng = argc - 2; // number of grammars
	std::unordered_map<std::string, int> sym_map;
	std::vector<Grammar> grammars;
	for (int i = 0; i < ng; i++) {
		grammars.push_back(parseGrammar(argv[2 + i], sym_map));
	}
	std::unordered_map<std::string, int> node_map;
	auto p = parseGraph(argv[2], sym_map, node_map);
	int nv = p.first;
	std::unordered_set<long long> edges = std::move(p.second);
	while (true) {
		int esize = edges.size();
		std::vector<Graph> graphs(ng);
		for (int i = 0; i < ng; i++) {
			graphs[i].clear();
			graphs[i].setNumberOfVertices(nv);
			graphs[i].addEdges(edges);
			auto summaries = graphs[i].runCFLReachability(grammars[i]);
			edges = graphs[i].getEdgeClosure(grammars[i], summaries);
		}
		if (edges.size() == esize) {
			int ctr = 0;
			for (int s = 0; s < nv; s++) {
				for (int t = 0; t < nv; t++) {
					bool ok = true;
					for (int i = 0; i < ng; i++) {
						ok &&= graphs[i].hasEdge(make_fast_triple(s, grammars[i].startSymbol, t));
					}
					if (ok) {
						ctr++;
					}
				}
			}
			std::cout << "MR: " << ctr << std::endl;
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc >= 3) {
		check_resource(run);
	} else {
		std::cerr << "Usage: " << argv[0] << " <graph-file-path> (<grammar-file-path>)+" << std::endl;
	}
}
