#include "grammar/grammar.h"
#include "graph/graph.h"
#include "hasher/hasher.h"
#include "parser/parser.h"
#include "lcllib/lcllib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <chrono>

void printResult(std::unordered_set<std::pair<int, int>, IntPairHasher> &reachablePairs, GraphFile &graphFile, std::vector<Graph> &graphs) {
	int ng = graphs.size();
	int ctr = 0;
	for (auto &p : reachablePairs) {
		bool ok = true;
		for (int i = 0; i < ng; i++) {
			if (!(graphs[i].hasEdge(std::make_tuple(p.first, graphFile.grammars[i].startSymbol, p.second)))) {
				ok = false;
				break;
			}
		}
		if (ok) {
			ctr++;
		}
	}
	std::cout << "Number of Reachable Pairs: " << ctr << std::endl;
}

void printUsage(const std::string &programName) {
	std::cerr << "Usage: " << programName << " <\"naive\"/\"refine\"> <graph-file-path>" << std::endl;
}

void run(int argc, char *argv[]) {
	if (argc == 3) {
		std::string option = argv[1];
		GraphFile gf = parseGraphFile(argv[2]);
		if (option == "naive") {
			// lcl
			std::stringstream buffer;
			for (auto &e : gf.edges) {
				buffer
					<< gf.nodeMapR[std::get<0>(e)] << "->" << gf.nodeMapR[std::get<2>(e)]
					<< "[label=\"" << gf.symMapR[std::get<1>(e)] << "\"]\n";
			}
			std::set<std::pair<std::string, std::string>> reachableStringPairs;
			std::set<std::tuple<std::string, std::string, std::string>> contributingStringEdges;
			runLCL(buffer, reachableStringPairs, false, contributingStringEdges);
			std::unordered_set<std::pair<int, int>, IntPairHasher> reachablePairs;
			for (auto &sp : reachableStringPairs) {
				reachablePairs.insert(std::make_pair(gf.nodeMap[sp.first], gf.nodeMap[sp.second]));
			}
			// cfl
			int ng = gf.grammars.size();
			int nv = gf.nodeMap.size();
			std::vector<Graph> graphs(ng);
			for (int i = 0; i < ng; i++) {
				graphs[i].reinit(nv, gf.edges);
				std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
				graphs[i].runCFLReachability(gf.grammars[i], false, record);
			}
			// print
			printResult(reachablePairs, gf, graphs);
		} else if (option == "refine") {
			// common
			std::unordered_set<Edge, EdgeHasher> edges = gf.edges;
			std::unordered_set<Edge, EdgeHasher>::size_type prev_size;
			// for lcl
			std::set<std::pair<std::string, std::string>> reachableStringPairs;
			std::set<std::tuple<std::string, std::string, std::string>> contributingStringEdges;
			// for cfl
			int ng = gf.grammars.size();
			int nv = gf.nodeMap.size();
			std::vector<Graph> graphs(ng);
			// main refinement loop
			do {
				prev_size = edges.size();
				// lcl
				std::stringstream buffer;
				for (auto &e : edges) {
					buffer
						<< gf.nodeMapR[std::get<0>(e)] << "->" << gf.nodeMapR[std::get<2>(e)]
						<< "[label=\"" << gf.symMapR[std::get<1>(e)] << "\"]\n";
				}
				runLCL(buffer, reachableStringPairs, true, contributingStringEdges);
				edges = std::unordered_set<Edge, EdgeHasher>();
				for (auto &se : contributingStringEdges) {
					edges.insert(
						std::make_tuple(gf.nodeMap[std::get<0>(se)], gf.symMap[std::get<1>(se)], gf.nodeMap[std::get<2>(se)])
					);
				}
				// cfl
				for (int i = 0; i < ng; i++) {
					graphs[i].reinit(nv, edges);
					std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
					auto summaries = graphs[i].runCFLReachability(gf.grammars[i], true, record);
					edges = graphs[i].getEdgeClosure(gf.grammars[i], summaries, record);
				}
			} while (edges.size() != prev_size);
			std::unordered_set<std::pair<int, int>, IntPairHasher> reachablePairs;
			for (auto &sp : reachableStringPairs) {
				reachablePairs.insert(std::make_pair(gf.nodeMap[sp.first], gf.nodeMap[sp.second]));
			}
			// print
			printResult(reachablePairs, gf, graphs);
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
