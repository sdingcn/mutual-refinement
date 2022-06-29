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

void run(int argc, char *argv[]) {
	// type aliases
	using Edge = std::tuple<int, int, int>;
	using EdgeHasher = IntTripleHasher;
	if (argc == 2) {
		GraphFile gf = parseGraphFile(sys.argv[1]);
		std::vector<Graph> graphs(gf.grammars.size());
#ifdef REFINE
		std::unordered_set<Edge, EdgeHasher> edges = gf.edges;
		std::unordered_set<Edge, EdgeHasher>::size_type prev_size;
		do {
			prev_size = edges.size();
			for (int i = 0; i < gf.grammars.size(); i++) {
				graphs[i].reinit(gf.nodeMap.size(), edges);
				std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
				auto summaries = graphs[i].runCFLReachability(gf.grammars[i], true, record);
				edges = graphs[i].getEdgeClosure(gf.grammars[i], summaries, record);
			}
		} while (edges.size() != prev_size);
#else
		for (int i = 0; i < gf.grammars.size(); i++) {
			graphs[i].reinit(gf.nodeMap.size(), gf.edges);
			graphs[i].addEdges(edges);
			std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
			graphs[i].runCFLReachability(grammars[i], false, record);
		}
#endif
		int ctr = 0;
		for (int s = 0; s < gf.nodeMap.size(); s++) {
			for (int t = 0; t < gf.nodeMap.size(); t++) {
				bool ok = true;
				for (int i = 0; i < gf.grammars.size(); i++) {
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
		std::cerr << "Usage: " << argv[0] << " <graph-file-path>" << std::endl;
	}
}

int main(int argc, char *argv[]) {
	// time
	auto start = std::chrono::steady_clock::now();
	run(argc, argv);
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	// space
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
	// print resource consumption
	std::cout
		<< "*** Resource Consumption ***" << std::endl
		<< "Total Time (Seconds): " << elapsed_seconds.count() << std::endl
		<< "Peak Space (kB): " << vmpeak << std::endl; 
}
