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
#include <tuple>
#include <chrono>
#include <cassert>
#include <utility>

void printUsage(const std::string &programName) {
	std::cerr << "Usage: " << programName << " <\"reduction\"/\"naive\"/\"refine\"> <graph-file-path>" << std::endl;
}

std::unordered_set<Edge, EdgeHasher> intersectResults(const std::vector<std::unordered_set<Edge, EdgeHasher>> &results) {
	int n = results.size();
	std::assert(n >= 1);
	auto r = results[0];
	for (int i = 1; i < n; i++) {
		std::unordered_set<Edge, EdgeHasher> nr;
		for (auto &e : r) {
			if (results[i].count(e) > 0) {
				nr.insert(e);
			}
		}
		r = std::move(nr);
	}
	return r;
}

struct RawGraph {
	int numNode;
	int numEdge;
	int numGrammar;
	std::unordered_map<std::string, int> nodeMap;
	std::unordered_map<int, std::string> nodeMapR;
	std::unordered_map<std::string, int> symMap;
	std::unordered_map<int, std::string> symMapR;
	std::vector<Grammar> grammars;
	std::unordered_set<Edge, EdgeHasher> edges;
};

RawGraph makeRawGraph(const std::vector<Line> &lines) {
#ifdef INTERDYCK
#else
#endif
}

void run(int argc, char *argv[]) {
	if (argc == 3) {
		std::string option = argv[1];
		std::vector<Line> lines = parseGraphFile(argv[2]);
		RawGraph rg = makeRawGraph(lines);
		if (option == "naive") {
			std::vector<Graph> graphs(rg.numGrammar);
			std::vector<std::unordered_set<Edge, EdgeHasher>> results(rg.numGrammar);
			for (int i = 0; i < ng; i++) {
				graphs[i].reinit(rg.numNode, rg.edges);
				std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
				results[i] = graphs[i].runCFLReachability(grammars[i], false, record);
			}
			// print
			std::cout << intersectResults(results).size() << std::endl;
		} else if (option == "refine") {
			std::unordered_set<Edge, EdgeHasher> edges = rg.edges;
			std::unordered_set<Edge, EdgeHasher>::size_type prev_size;
			std::vector<Graph> graphs(rg.numGrammar);
			std::vector<std::unordered_set<Edge, EdgeHasher>> results(rg.numGrammar);
			// main refinement loop
			do {
				prev_size = edges.size();
				for (int i = 0; i < ng; i++) {
					graphs[i].reinit(rg.numNode, rg.edges);
					std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
					results[i] = graphs[i].runCFLReachability(grammars[i], true, record);
					edges = graphs[i].getEdgeClosure(grammars[i], results[i], record);
				}
			} while (edges.size() != prev_size);
			// print
			std::cout << intersectResults(results).size() << std::endl;
		} else {
			printUsage(argv[0]);
		}
	} else {
		printUsage(argv[0]);
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
	// print
	std::cout
		<< "*** Resource Consumption ***" << std::endl
		<< "Total Time (Seconds): " << elapsed_seconds.count() << std::endl
		<< "Peak Space (kB): " << vmpeak << std::endl; 
}
