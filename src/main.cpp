#include "grammar/grammar.h"
#include "graph/graph.h"
#include "parser/parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#ifdef ROBIN
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
template <typename K, typename V> using hash_map = tsl::robin_map<K, V>;
template <typename T> using hash_set = tsl::robin_set<T>;
#else
#include <unordered_map>
#include <unordered_set>
template <typename K, typename V> using hash_map = std::unordered_map<K, V>;
template <typename T> using hash_set = std::unordered_set<T>;
#endif
#include <utility>
#include <chrono>

int main(int argc, char *argv[]) {
	// start time
	auto start = std::chrono::steady_clock::now();
	// main body
	if (argc == 2) {
#ifdef NAIVE
		hash_map<std::string, int> sym_map;
		std::vector<Grammar> grammars = extractDyck(argv[1], sym_map);
		int ng = grammars.size();
		hash_map<std::string, int> node_map;
		auto tmp = parseGraph(argv[1], sym_map, node_map);
		int nv = tmp.first;
		hash_set<long long> edges = std::move(tmp.second);
		std::vector<Graph> graphs(ng);
		for (int i = 0; i < ng; i++) {
			graphs[i].init(nv);
			graphs[i].addEdges(edges);
			hash_map<long long, hash_set<long long>> record;
			graphs[i].runCFLReachability(grammars[i], false, record);
		}
		int ctr = 0;
		for (int s = 0; s < nv; s++) {
			for (int t = 0; t < nv; t++) {
				bool ok = true;
				for (int i = 0; i < ng; i++) {
					if (!(graphs[i].hasEdge(make_fast_triple(s, grammars[i].startSymbol, t)))) {
						ok = false;
						break;
					}
				}
				if (ok) {
					ctr++;
				}
			}
		}
		std::cout << "naive: " << ctr << std::endl;
#endif
#ifdef REFINE
		hash_map<std::string, int> sym_map;
		std::vector<Grammar> grammars = extractDyck(argv[1], sym_map);
		int ng = grammars.size();
		hash_map<std::string, int> node_map;
		auto tmp = parseGraph(argv[1], sym_map, node_map);
		int nv = tmp.first;
		hash_set<long long> edges = std::move(tmp.second);
		hash_set<long long>::size_type prev_size;
		std::vector<Graph> graphs(ng);
		do {
			prev_size = edges.size();
			for (int i = 0; i < ng; i++) {
				graphs[i].init(nv);
				graphs[i].addEdges(edges);
				hash_map<long long, hash_set<long long>> record;
				auto summaries = graphs[i].runCFLReachability(grammars[i], true, record);
				edges = graphs[i].getEdgeClosure(grammars[i], summaries, record);
			}
		} while (edges.size() != prev_size);
		int ctr = 0;
		for (int s = 0; s < nv; s++) {
			for (int t = 0; t < nv; t++) {
				bool ok = true;
				for (int i = 0; i < ng; i++) {
					if (!(graphs[i].hasEdge(make_fast_triple(s, grammars[i].startSymbol, t)))) {
						ok = false;
						break;
					}
				}
				if (ok) {
					ctr++;
				}
			}
		}
		std::cout << "refine: " << ctr << std::endl;
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
