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
	std::cerr << "Usage: " << programName << " <graph-file> <\"naive\"/\"refine\"> <\"taint\"/\"valueflow\">" << std::endl;
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

RawGraph makeRawGraph(const std::vector<Line> &lines, const std::string &analysis) {
	// number nodes
	std::vector<std::string> nodes;
	for (auto &line : lines) {
		nodes.push_back(std::get<0>(line));
		nodes.push_back(std::get<2>(line));
	}
	std::unordered_map<std::string, int> nodeMap = number(nodes);
	std::unordered_map<int, std::string> nodeMapR = reverseMap(nodeMap);
	// number dycks
	std::unordered_map<std::string, std::unordered_set<std::string>> dyckNumbers;
	for (auto &line: lines) {
		auto symbol = std::get<1>(line);
		if (symbol[0] == 'o' || symbol[0] == 'c') {
			std::string dyck = symbol.substr(1, 1);
			std::string parNumber = symbol.substr(4, symbol.size() - 4);
			dyckNumbers[dyck].insert(parNumber);
		}
	}
	// two cases
	if (analysis == "taint") {
		/**********
		 * number symbols
		 **********/
		std::vector<std::string> symbols;
		symbols.push_back("dp");
		for (auto parNumber : dyckNumbers["p"]) {
			symbols.push_back("ip--" + parNumber);
			symbols.push_back("op--" + parNumber);
			symbols.push_back("cp--" + parNumber);
		}
		symbols.push_back("lb");
		symbols.push_back("db");
		for (auto parNumber : dyckNumbers["b"]) {
			symbols.push_back("ib--" + parNumber);
			symbols.push_back("ob--" + parNumber);
			symbols.push_back("cb--" + parNumber);
		}
		std::unordered_map<std::string, int> symMap = number(symbols);
		std::unordered_map<int, std::string> symMapR = reverseMap(symMap);
		/**********
		 * construct grammars
		 **********/
		std::vector<Grammar> grammars;
		/* grammar 1 (parenthesis) */
		{
			Grammar gm;
			// terminals
			for (auto &n : dyckNumbers["p"]) {
				gm.addTerminal(symMap["op--" + n]);
				gm.addTerminal(symMap["cp--" + n]);
			}
			for (auto &n : dyckNumbers["b"]) {
				gm.addTerminal(symMap["ob--" + n]);
				gm.addTerminal(symMap["cb--" + n]);
			}
			// nonterminals
			gm.addNonterminal(symMap["dp"]);
			for (auto &n : dyckNumbers["p"]) {
				gm.addNonterminal(symMap["ip--" + n]);
			}
			// productions
			gm.addEmptyProduction(symMap["dp"]);
			gm.addBinaryProduction(symMap["dp"], symMap["dp"], symMap["dp"]);
			for (auto &n : dyckNumbers["p"]) {
				gm.addBinaryProduction(symMap["dp"], symMap["op--" + n], symMap["ip--" + n]);
				gm.addBinaryProduction(symMap["ip--" + n], symMap["dp"], symMap["cp--" + n]);
			}
			for (auto &n : dyckNumbers["b"]) {
				gm.addUnaryProduction(symMap["dp"], symMap["ob--" + n]);
				gm.addUnaryProduction(symMap["dp"], symMap["cb--" + n]);
			}
			// start symbol
			gm.addStartSymbol(symMap["dp"]);
			// finalize
			gm.initFastIndices();
			grammars.push_back(std::move(gm));
		}
		/* grammar 2 (brackets) */
		{
			Grammar gm;
			// terminals
			for (auto &n : dyckNumbers["p"]) {
				gm.addTerminal(symMap["op--" + n]);
				gm.addTerminal(symMap["cp--" + n]);
			}
			for (auto &n : dyckNumbers["b"]) {
				gm.addTerminal(symMap["ob--" + n]);
				gm.addTerminal(symMap["cb--" + n]);
			}
			// nonterminals
			gm.addNonterminal(symMap["lb"]);
			gm.addNonterminal(symMap["db"]);
			for (auto &n : dyckNumbers["b"]) {
				gm.addNonterminal(symMap["ib--" + n]);
			}
			// productions
			gm.addEmptyProduction(symMap["db"]);
			gm.addBinaryProduction(symMap["db"], symMap["db"], symMap["db"]);
			for (auto &n : dyckNumbers["b"]) {
				gm.addBinaryProduction(symMap["db"], symMap["ob--" + n], symMap["ib--" + n]);
				gm.addBinaryProduction(symMap["ib--" + n], symMap["db"], symMap["cb--" + n]);
			}
			for (auto &n : dyckNumbers["p"]) {
				gm.addUnaryProduction(symMap["db"], symMap["op--" + n]);
				gm.addUnaryProduction(symMap["db"], symMap["cp--" + n]);
			}
			gm.addBinaryProduction(symMap["lb"], symMap["db"], symMap["lb"]);
			for (auto &n : dyckNumbers["b"]) {
				gm.addBinaryProduction(symMap["lb"], symMap["ob--" + n], symMap["lb"]);
			}
			gm.addEmptyProduction(symMap["lb"]);
			// start symbol
			gm.addStartSymbol(symMap["lb"]);
			// finalize
			gm.initFastIndices();
			grammars.push_back(std::move(gm));
		}
		/**********
		 * construct edges
		 **********/
		std::unordered_set<Edge, EdgeHasher> edges;
		for (auto &line : lines) {
			edges.insert(std::make_tuple(
				nodeMap[std::get<0>(line)],
				symMap[std::get<1>(line)],
				nodeMap[std::get<2>(line)]
			));
		}
		/**********
		 * return
		 **********/
		RawGraph rg;
		rg.numNode = nodeMap.size();
		rg.numEdge = edges.size();
		rg.numGrammar = grammars.size();
		rg.nodeMap = nodeMap;
		rg.nodeMapR = nodeMapR;
		rg.symMap = symMap;
		rg.symMapR = symMapR;
		rg.grammars = grammars;
		rg.edges = edges;
		return rg;
	} else {
		assert(analysis == "valueflow");
		/**********
		 * number symbols
		 **********/
		std::vector<std::string> symbols;
		symbols.push_back("normal");
		symbols.push_back("dp");
		for (auto parNumber : dyckNumbers["p"]) {
			symbols.push_back("ip--" + parNumber);
			symbols.push_back("op--" + parNumber);
			symbols.push_back("cp--" + parNumber);
		}
		symbols.push_back("db");
		for (auto parNumber : dyckNumbers["b"]) {
			symbols.push_back("ib--" + parNumber);
			symbols.push_back("ob--" + parNumber);
			symbols.push_back("cb--" + parNumber);
		}
		symbols.push_back("en");
		symbols.push_back("ien");
		symbols.push_back("g");
		std::unordered_map<std::string, int> symMap = number(symbols);
		std::unordered_map<int, std::string> symMapR = reverseMap(symMap);
		/**********
		 * construct grammars
		 **********/
		std::vector<Grammar> grammars;
		/* grammar 1 (parenthesis) */
		{
			Grammar gm;
			// terminals
			gm.addTerminal(symMap["normal"]);
			for (auto &n : dyckNumbers["p"]) {
				gm.addTerminal(symMap["op--" + n]);
				gm.addTerminal(symMap["cp--" + n]);
			}
			for (auto &n : dyckNumbers["b"]) {
				gm.addTerminal(symMap["ob--" + n]);
				gm.addTerminal(symMap["cb--" + n]);
			}
			// nonterminals
			gm.addNonterminal(symMap["dp"]);
			for (auto &n : dyckNumbers["p"]) {
				gm.addNonterminal(symMap["ip--" + n]);
			}
			// productions
			gm.addEmptyProduction(symMap["dp"]);
			gm.addBinaryProduction(symMap["dp"], symMap["dp"], symMap["dp"]);
			for (auto &n : dyckNumbers["p"]) {
				gm.addBinaryProduction(symMap["dp"], symMap["op--" + n], symMap["ip--" + n]);
				gm.addBinaryProduction(symMap["ip--" + n], symMap["dp"], symMap["cp--" + n]);
			}
			for (auto &n : dyckNumbers["b"]) {
				gm.addUnaryProduction(symMap["dp"], symMap["ob--" + n]);
				gm.addUnaryProduction(symMap["dp"], symMap["cb--" + n]);
			}
			gm.addUnaryProduction(symMap["dp"], symMap["normal"]);
			// start symbol
			gm.addStartSymbol(symMap["dp"]);
			// finalize
			gm.initFastIndices();
			grammars.push_back(std::move(gm));
		}
		/* grammar 2 (brackets) */
		{
			Grammar gm;
			// terminals
			gm.addTerminal(symMap["normal"]);
			for (auto &n : dyckNumbers["p"]) {
				gm.addTerminal(symMap["op--" + n]);
				gm.addTerminal(symMap["cp--" + n]);
			}
			for (auto &n : dyckNumbers["b"]) {
				gm.addTerminal(symMap["ob--" + n]);
				gm.addTerminal(symMap["cb--" + n]);
			}
			// nonterminals
			gm.addNonterminal(symMap["db"]);
			for (auto &n : dyckNumbers["b"]) {
				gm.addNonterminal(symMap["ib--" + n]);
			}
			// productions
			gm.addEmptyProduction(symMap["db"]);
			gm.addBinaryProduction(symMap["db"], symMap["db"], symMap["db"]);
			for (auto &n : dyckNumbers["b"]) {
				gm.addBinaryProduction(symMap["db"], symMap["ob--" + n], symMap["ib--" + n]);
				gm.addBinaryProduction(symMap["ib--" + n], symMap["db"], symMap["cb--" + n]);
			}
			for (auto &n : dyckNumbers["p"]) {
				gm.addUnaryProduction(symMap["db"], symMap["op--" + n]);
				gm.addUnaryProduction(symMap["db"], symMap["cp--" + n]);
			}
			gm.addUnaryProduction(symMap["db"], symMap["normal"]);
			// start symbol
			gm.addStartSymbol(symMap["db"]);
			// finalize
			gm.initFastIndices();
			grammars.push_back(std::move(gm));
		}
		/* grammar 3 (endpoints) */
		{
			Grammar gm;
			// terminals
			gm.addTerminal(symMap["normal"]);
			for (auto &n : dyckNumbers["p"]) {
				gm.addTerminal(symMap["op--" + n]);
				gm.addTerminal(symMap["cp--" + n]);
			}
			for (auto &n : dyckNumbers["b"]) {
				gm.addTerminal(symMap["ob--" + n]);
				gm.addTerminal(symMap["cb--" + n]);
			}
			// nonterminals
			gm.addNonterminal(symMap["en"]);
			gm.addNonterminal(symMap["ien"]);
			gm.addNonterminal(symMap["g"]);
			// productions
			for (auto &n : dyckNumbers["b"]) {
				gm.addBinaryProduction(symMap["en"], symMap["ob--" + n], symMap["ien"]);
				gm.addBinaryProduction(symMap["ien"], symMap["g"], symMap["cb--" + n]);
			}
			gm.addEmptyProduction(symMap["g"]);
			for (auto &t : gm.terminals) {
				gm.addUnaryProduction(symMap["g"], t);
			}
			// start symbol
			gm.addStartSymbol(symMap["en"]);
			// finalize
			gm.initFastIndices();
			grammars.push_back(std::move(gm));
		}
		/**********
		 * construct edges
		 **********/
		std::unordered_set<Edge, EdgeHasher> edges;
		for (auto &line : lines) {
			edges.insert(std::make_tuple(
				nodeMap[std::get<0>(line)],
				symMap[std::get<1>(line)],
				nodeMap[std::get<2>(line)]
			));
		}
		/**********
		 * return
		 **********/
		RawGraph rg;
		rg.numNode = nodeMap.size();
		rg.numEdge = edges.size();
		rg.numGrammar = grammars.size();
		rg.nodeMap = nodeMap;
		rg.nodeMapR = nodeMapR;
		rg.symMap = symMap;
		rg.symMapR = symMapR;
		rg.grammars = grammars;
		rg.edges = edges;
		return rg;
	}
}

void run(int argc, char *argv[]) {
	if (argc == 4) {
		// get arguments
		std::string file = argv[1];
		std::string option = argv[2];
		std::string analysis = argv[3];
		// construct the raw graph
		std::vector<Line> lines = parseGraphFile(file);
		const RawGraph rg = makeRawGraph(lines, analysis);
		if (option == "naive") {
			std::vector<Graph> graphs(rg.numGrammar);
			std::vector<std::unordered_set<Edge, EdgeHasher>> results(rg.numGrammar);
			for (int i = 0; i < rg.numGrammar; i++) {
				graphs[i].reinit(rg.numNode, rg.edges);
				std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
				results[i] = graphs[i].runCFLReachability(rg.grammars[i], false, record);
			}
			// print
			std::cout << "Number of Reachable Pairs: " << intersectResults(results).size() << std::endl;
		} else if (option == "refine") {
			std::unordered_set<Edge, EdgeHasher> edges = rg.edges;
			std::unordered_set<Edge, EdgeHasher>::size_type prev_size;
			std::vector<Graph> graphs(rg.numGrammar);
			std::vector<std::unordered_set<Edge, EdgeHasher>> results(rg.numGrammar);
			int refineIter = 0;
			// main refinement loop
			do {
				prev_size = edges.size();
				for (int i = 0; i < rg.numGrammar; i++) {
					graphs[i].reinit(rg.numNode, edges);
					std::unordered_map<Edge, std::unordered_set<Edge, EdgeHasher>, EdgeHasher> record;
					results[i] = graphs[i].runCFLReachability(rg.grammars[i], true, record);
					edges = graphs[i].getEdgeClosure(rg.grammars[i], results[i], record);
				}
				refineIter++;
			} while (edges.size() != prev_size);
			// print
			std::cout << "Number of Refinement Iterations: " << refineIter << std::endl;
			std::cout << "Number of Reachable Pairs: " << intersectResults(results).size() << std::endl;
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
