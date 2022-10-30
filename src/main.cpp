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
	std::cerr << "Usage: " << programName << " <graph-file> <\"naive\"/\"refine\"> <0-3(pud)> <0-3(bud)>" << std::endl;
}

std::unordered_set<std::pair<int, int>, IntPairHasher>
	intersectResults(const std::vector<std::unordered_set<Edge, EdgeHasher>> &results) {
	int n = results.size();
	assert(n >= 1);
	std::vector<std::unordered_set<std::pair<int, int>, IntPairHasher>> pairsets(n);
	for (int i = 0; i < n; i++) {
		for (auto &e : results[i]) {
			pairsets[i].insert(std::make_pair(std::get<0>(e), std::get<2>(e)));
		}
	}
	std::unordered_set<std::pair<int, int>, IntPairHasher> ps = pairsets[0];
	for (int i = 1; i < n; i++) {
		std::unordered_set<std::pair<int, int>, IntPairHasher> tmp;
		for (auto &p : ps) {
			if (pairsets[i].count(p) > 0) {
				tmp.insert(p);
			}
		}
		ps = std::move(tmp);
	}
	return ps;
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

RawGraph makeRawGraph(const std::vector<Line> &lines, int pud, int bud) {
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
		if (symbol != "normal") {
			std::string dyck = symbol.substr(1, 1);
			std::string parNumber = symbol.substr(4, symbol.size() - 4);
			dyckNumbers[dyck].insert(parNumber);
		}
	}
	std::vector<std::string> symbols;
	symbols.push_back("normal");
	for (auto dyck : std::vector<std::string>{"p", "b"}) {
		symbols.push_back("d" + dyck);
		symbols.push_back("l" + dyck);
		symbols.push_back("r" + dyck);
		symbols.push_back("s" + dyck);
		for (auto parNumber : dyckNumbers[dyck]) {
			symbols.push_back("i" + dyck + "--" + parNumber);
			symbols.push_back("o" + dyck + "--" + parNumber);
			symbols.push_back("c" + dyck + "--" + parNumber);
		}
	}
	std::unordered_map<std::string, int> symMap = number(symbols);
	std::unordered_map<int, std::string> symMapR = reverseMap(symMap);
	// construct grammars
	// unmatchDegree: 0 for (), 1 for ), 2 for (, 3 for )(
	auto construct = [&dyckNumbers, &symMap]
	(const std::string &dyck, const std::string &otherDyck, int unmatchDegree) -> Grammar {
		Grammar gm;
		// terminals
		gm.addTerminal(symMap["normal"]);
		for (auto &n : dyckNumbers[dyck]) {
			gm.addTerminal(symMap["o" + dyck + "--" + n]);
			gm.addTerminal(symMap["c" + dyck + "--" + n]);
		}
		for (auto &n : dyckNumbers[otherDyck]) {
			gm.addTerminal(symMap["o" + otherDyck + "--" + n]);
			gm.addTerminal(symMap["c" + otherDyck + "--" + n]);
		}
		// nonterminals
		gm.addNonterminal(symMap["d" + dyck]);
		for (auto &n : dyckNumbers[dyck]) {
			gm.addNonterminal(symMap["i" + dyck + "--" + n]);
		}
		if (unmatchDegree & 1) gm.addNonterminal(symMap["l" + dyck]);
		if (unmatchDegree & 2) gm.addNonterminal(symMap["r" + dyck]);
		if (unmatchDegree == 3) gm.addNonterminal(symMap["s" + dyck]);
		/* begin grammar */
		// d      -> empty
		gm.addEmptyProduction(symMap["d" + dyck]);
		// d      -> d d
		gm.addBinaryProduction(symMap["d" + dyck], symMap["d" + dyck], symMap["d" + dyck]);
		// d      -> o--[n] i--[n]
		// i--[n] -> d      c--[n]
		for (auto &n : dyckNumbers[dyck]) {
			gm.addBinaryProduction(
					symMap["d" + dyck],
					symMap["o" + dyck + "--" + n],
					symMap["i" + dyck + "--" + n]);
			gm.addBinaryProduction(
					symMap["i" + dyck + "--" + n],
					symMap["d" + dyck],
					symMap["c" + dyck + "--" + n]);
		}
		// d      -> ...
		gm.addUnaryProduction(symMap["d" + dyck], symMap["normal"]);
		for (auto &n : dyckNumbers[otherDyck]) {
			gm.addUnaryProduction(
					symMap["d" + dyck],
					symMap["o" + otherDyck + "--" + n]);
			gm.addUnaryProduction(
					symMap["d" + dyck],
					symMap["c" + otherDyck + "--" + n]);
		}
		if (unmatchDegree & 1) {
			// l      -> d      l
			gm.addBinaryProduction(symMap["l" + dyck], symMap["d" + dyck], symMap["l" + dyck]);
			// l      -> o--[n] l
			for (auto &n : dyckNumbers[dyck]) {
				gm.addBinaryProduction(
						symMap["l" + dyck],
						symMap["o" + dyck + "--" + n],
						symMap["l" + dyck]);
			}
			// l      -> empty
			gm.addEmptyProduction(symMap["l" + dyck]);
		}
		if (unmatchDegree & 2) {
			// r      -> r      d
			gm.addBinaryProduction(symMap["r" + dyck], symMap["r" + dyck], symMap["d" + dyck]);
			// r      -> r c--[n]
			for (auto &n : dyckNumbers[dyck]) {
				gm.addBinaryProduction(
						symMap["r" + dyck],
						symMap["r" + dyck],
						symMap["c" + dyck + "--" + n]);
			}
			// r      -> empty
			gm.addEmptyProduction(symMap["r" + dyck]);
		}
		if (unmatchDegree == 3) {
			// s      -> l r
			gm.addBinaryProduction(symMap["s" + dyck], symMap["l" + dyck], symMap["r" + dyck]);
		}
		/* end grammar */
		// start symbol
		if (unmatchDegree == 0) {
			gm.addStartSymbol(symMap["d" + dyck]);
		} else if (unmatchDegree == 1) {	
			gm.addStartSymbol(symMap["l" + dyck]);
		} else if (unmatchDegree == 2) {
			gm.addStartSymbol(symMap["r" + dyck]);
		} else if (unmatchDegree == 3) {
			gm.addStartSymbol(symMap["s" + dyck]);
		}
		gm.initFastIndices();
		return gm;
	};
	std::vector<Grammar> grammars{construct("p", "b", pud), construct("b", "p", bud)};
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
	if (argc == 5) {
		// get arguments
		std::string file = argv[1];
		std::string option = argv[2];
		int pud = std::stoi(std::string(argv[3]));
		int bud = std::stoi(std::string(argv[4]));
		// construct the raw graph
		std::vector<Line> lines = parseGraphFile(file);
		const RawGraph rg = makeRawGraph(lines, pud, bud);
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
		}
	} else {
		printUsage(argv[0]);
	}
}

int main(int argc, char *argv[]) {
	// time
	auto start = std::chrono::steady_clock::now();
	run(argc, argv); // the real run is here
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
