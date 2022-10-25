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
	std::cerr << "Usage: " << programName << " <\"naive\"/\"refine\"> <graph-file-path>" << std::endl;
}

std::unordered_set<std::pair<int, int>, IntPairHasher>
	intersectResults(const std::vector<std::unordered_set<Edge, EdgeHasher>> &results) {
	int n = results.size();
	assert(n >= 1);
	std::unordered_set<std::pair<int, int>, IntPairHasher> r0;
	for (auto &e : results[0]) {
		r0.insert(std::make_pair(std::get<0>(e), std::get<2>(e)));
	}
	for (int i = 1; i < n; i++) {
		std::unordered_set<std::pair<int, int>, IntPairHasher> r1;
		for (auto &e : results[i]) {
			if (r0.count(std::make_pair(std::get<0>(e), std::get<2>(e))) > 0) {
				r1.insert(std::make_pair(std::get<0>(e), std::get<2>(e)));
			}
		}
		r0 = std::move(r1);
	}
	return r0;
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

std::unordered_map<std::string, int> number(const std::vector<std::string> &names) {
	std::unordered_map<std::string, int> mp;
	int n = 0;
	for (auto &name : names) {
		if (mp.count(name) == 0) {
			mp[name] = n++;
		}
	}
	return mp;
}

template <typename T, typename U>
std::unordered_map<U, T> reverseMap(const std::unordered_map<T, U> &mp) {
	std::unordered_map<U, T> mpR;
	for (auto &kv : mp) {
		mpR[kv.second] = kv.first;
	}
	return mpR;
}

RawGraph makeRawGraph(const std::vector<Line> &lines) {
	// number nodes
	std::vector<std::string> nodes;
	for (auto &line : lines) {
		nodes.push_back(std::get<0>(line));
		nodes.push_back(std::get<2>(line));
	}
	std::unordered_map<std::string, int> nodeMap = number(nodes);
	std::unordered_map<int, std::string> nodeMapR = reverseMap(nodeMap);
	// number symbols
	std::unordered_map<std::string, std::unordered_set<std::string>> dyckNumbers;
	for (auto &line: lines) {
		auto symbol = std::get<1>(line);
		std::string dyck = symbol.substr(1, 1);
		std::string parNumber = symbol.substr(4, symbol.size() - 4);
		dyckNumbers[dyck].insert(parNumber);
	}
	std::vector<std::string> symbols;
	for (auto dyck : std::vector<std::string>{"p", "b"}) {
		symbols.push_back("d" + dyck);
		for (auto parNumber : dyckNumbers[dyck]) {
			symbols.push_back("d" + dyck + "--" + parNumber);
			symbols.push_back("o" + dyck + "--" + parNumber);
			symbols.push_back("c" + dyck + "--" + parNumber);
		}
	}
	std::unordered_map<std::string, int> symMap = number(symbols);
	std::unordered_map<int, std::string> symMapR = reverseMap(symMap);
	// construct grammars
	auto construct = [&dyckNumbers, &symMap]
	(const std::string &dyck, const std::string &otherDyck) -> Grammar {
		Grammar gm;
		for (auto &n : dyckNumbers[dyck]) {
			gm.addTerminal(symMap["o" + dyck + "--" + n]);
			gm.addTerminal(symMap["c" + dyck + "--" + n]);
		}
		for (auto &n : dyckNumbers[otherDyck]) {
			gm.addTerminal(symMap["o" + otherDyck + "--" + n]);
			gm.addTerminal(symMap["c" + otherDyck + "--" + n]);
		}
		gm.addNonterminal(symMap["d" + dyck]);
		for (auto &n : dyckNumbers[dyck]) {
			gm.addNonterminal(symMap["d" + dyck + "--" + n]);
		}
		// d      -> empty
		gm.addEmptyProduction(symMap["d" + dyck]);
		// d      -> d d
		gm.addBinaryProduction(symMap["d" + dyck], symMap["d" + dyck], symMap["d" + dyck]);
		// d      -> o--[n] d--[n]
		// d--[n] -> d      c--[n]
		for (auto &n : dyckNumbers[dyck]) {
			gm.addBinaryProduction(
					symMap["d" + dyck],
					symMap["o" + dyck + "--" + n],
					symMap["d" + dyck + "--" + n]);
			gm.addBinaryProduction(
					symMap["d" + dyck + "--" + n],
					symMap["d" + dyck],
					symMap["c" + dyck + "--" + n]);
		}
		// d      -> ...
		for (auto &n : dyckNumbers[otherDyck]) {
			gm.addUnaryProduction(
					symMap["d" + dyck],
					symMap["o" + otherDyck + "--" + n]);
			gm.addUnaryProduction(
					symMap["d" + dyck],
					symMap["c" + otherDyck + "--" + n]);
		}
		gm.addStartSymbol(symMap["d" + dyck]);
		gm.initFastIndices();
		return gm;
	};
	std::vector<Grammar> grammars{construct("p", "b"), construct("b", "p")};
	// construct edges
	std::unordered_set<Edge, EdgeHasher> edges;
	for (auto &line : lines) {
		edges.insert(std::make_tuple(
			nodeMap[std::get<0>(line)],
			symMap[std::get<1>(line)],
			nodeMap[std::get<2>(line)]
		));
	}
	// return
	RawGraph rg;
	rg.numNode = nodeMap.size();
	rg.numEdge = edges.size();
	rg.numGrammar = 2;
	rg.nodeMap = nodeMap;
	rg.nodeMapR = nodeMapR;
	rg.symMap = symMap;
	rg.symMapR = symMapR;
	rg.grammars = grammars;
	rg.edges = edges;
	return rg;
}

void run(int argc, char *argv[]) {
	if (argc == 3) {
		std::string option = argv[1];
		std::vector<Line> lines = parseGraphFile(argv[2]);
		const RawGraph rg = makeRawGraph(lines);
		if (option == "naive") {
			std::vector<Graph> graphs(rg.numGrammar);
			std::vector<std::unordered_set<Edge, EdgeHasher>> results(rg.numGrammar);
			for (int i = 0; i < rg.numGrammar; i++) {
				graphs[i].reinit(rg.numNode, rg.edges);
				std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
				results[i] = graphs[i].runCFLReachability(rg.grammars[i], false, record);
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
				for (int i = 0; i < rg.numGrammar; i++) {
					graphs[i].reinit(rg.numNode, edges);
					std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
					results[i] = graphs[i].runCFLReachability(rg.grammars[i], true, record);
					edges = graphs[i].getEdgeClosure(rg.grammars[i], results[i], record);
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
