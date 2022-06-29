#include "parser.h"
#include "../hasher/hasher.h"
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

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

using Line = std::tuple<std::string, std::string, std::string>;

std::vector<Line> readLines(const std::string &fName) {
	std::ifstream in(fName); // file auto closed via destructor
	std::vector<Line> lines;
	std::string rawLine;
	while (getline(in, rawLine)) {
		if (rawLine.find("->") != std::string::npos) {
			// node1->node2[label="label"]
			std::string::size_type p1 = rawLine.find("->");
			std::string::size_type p2 = rawLine.find('[');
			std::string::size_type p3 = rawLine.find('=');
			std::string::size_type p4 = rawLine.find(']');
			lines.push_back(
				std::make_tuple(
					rawLine.substr(0, p1 - 0),
					rawLine.substr(p3 + 2, p4 - 1 - (p3 + 2)),
					rawLine.substr(p1 + 2, p2 - (p1 + 2))
				)
			);
		}
	}
	return lines;
}

GraphFile parseGraphFile(const std::string &fName) {
	std::vector<Line> lines = readLines(fName);
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
	GraphFile gf;
	gf.nodeMap = nodeMap;
	gf.nodeMapR = nodeMapR;
	gf.symMap = symMap;
	gf.symMapR = symMapR;
	gf.grammars = grammars;
	gf.edges = edges;
	return gf;
}
