#include "parser.h"
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <tuple>
#include "../common.h"

// line -> contains(line, '>')
bool isEdgeLine(const std::string &line) {
	for (char c : line) {
		if (c == '>') {
			return true;
		}
	}
	return false;
}

// line -> ((vertex1, vertex2), label)
std::pair<std::pair<std::string, std::string>, std::string> parsePALine(const std::string &line) {
	// 100->200[label="cp--10"]
	std::string::size_type p1 = line.find("->");
	std::string::size_type p2 = line.find('[');
	std::string::size_type p3 = line.find(']');
	return std::make_pair(std::make_pair(line.substr(0, p1 - 0), line.substr(p1 + 2, p2 - (p1 + 2))), p2 + 8, p3 - 1 - (p2 + 8));
}

std::tuple<
	std::map<std::string, int>,
	std::map<std::string, int>,
	std::vector<long long>,
	int,
	std::vector<Grammar>
> parsePAGraph(const std::string &fname) {
	std::ifstream in(fname); // file auto closed via destructor

	// read raw edges
	std::string line;
	std::vector<std::pair<std::pair<std::string, std::string>, std::string>> rawEdges;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			rawEdges.push_back(parsePALine(line));
		}
	}

	// encode original vertices and labels
	std::unordered_map<std::string, int> v_map;
	int v_ctr;
	std::unordered_map<std::string, int> l_map;
	int l_ctr;
	for (auto &ijl : rawEdges) {
		if (v_map.count(ijl.first.first) == 0) {
			v_map[ijl.first.first] = v_ctr++;
		}
		if (v_map.count(ijl.first.second) == 0) {
			v_map[ijl.first.second] = v_ctr++;
		}
		if (l_map.count(ijl.second) == 0) {
			l_map[ijl.second] = l_ctr++;
		}
	}

	// find out numbers for each Dyck
	std::unordered_map<std::string, std::unordered_set<std::string>> numbers;
	for (auto &p : l_map) {
		if (getDyck(p.first) == "op" || getDyck(p.first) == "cp") {
			numbers["p"].insert(p.first.substr(0, 2));
		}
	}
	for (auto &p : l_map) {
		if (getDyck(p.first) == "ob" || getDyck(p.first) == "cb") {
			numbers["b"].insert(p.first.substr(4, p.first.size() - 4));
		}
	}

	// encode nonterminals
	l_map["dp"] = l_ctr++;
	for (auto &s : ps) {
		l_map["dp--" + s] = l_ctr++;
	}
	l_map["db"] = l_ctr++;
	for (auto &s : bs) {
		l_map["db--" + s] = l_ctr++;
	}

	// grammar constructor
	auto construct_grammar = [&numbers, &l_map](Grammar &gm, const std::string &dyck, const std::string &other_dyck) -> void {
		for (auto &s : numbers[dyck]) {
			gm.terminals.insert(l_map["o" + dyck + "--" + s]);
			gm.terminals.insert(l_map["c" + dyck + "--" + s]);
		}
		for (auto &s : number[other_dyck]) {
			gm.terminals.insert(l_map["o" + other_dyck + "--" + s]);
			gm.terminals.insert(l_map["c" + other_dyck + "--" + s]);
		}
		gm.nonterminals.insert(l_map["d" + dyck]);
		for (auto &s : numbers[dyck]) {
			gm.nonterminals.insert(l_map["d" + dyck + "--" + s]);
		}
		// d      -> empty
		gm.emptyProductions.push_back(l_map["d" + dyck]);
		// d      -> d d
		gm.binaryProductions.push_back(std::make_pair(l_map["d" + dyck], std::make_pair(l_map["d" + dyck], l_map["d" + dyck])));
		// d      -> o--[s] d--[s]
		// d--[s] -> d      c--[s]
		for (auto &s : numbers[dyck]) {
			gm.binaryProductions.push_back(std::make_pair(
						l_map["d" + dyck],
						std::make_pair(l_map["o" + dyck + "--" + s], l_map["d" + dyck + "--" + s])
						));
			gm.binaryProductions.push_back(std::make_pair(
						l_map["d" + dyck + "--" + s],
						std::make_pair(l_map["d" + dyck], l_map["c" + dyck + "--" + s])
						));
		}
		// d      -> ...
		for (auto &s : numbers[other_dyck]) {
			gm.unaryProductions.push_back(std::make_pair(l_map["d" + dyck], l_map["o" + other_dyck + "--" + s]));
			gm.unaryProductions.push_back(std::make_pair(l_map["d" + dyck], l_map["c" + other_dyck + "--" + s]));
		}
		gm.startSymbol = l_map["d" + dyck];
		gm.fillInv(l_map.size());
	};

	// construct grammars
	Grammar gmp, gmb;
	construct_grammar(gmp, "p", "b");
	construct_grammar(gmb, "b", "p");

	// check boundaries
	if (v_map.size() - 1 > static_cast<int>(MASK)) {
		std::cerr << "Error: The graph contains too many nodes." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	if (l_map.size() - 1 > static_cast<int>(MASK)) {
		std::cerr << "Error: The grammar contains too many symbols." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// construct the normalized graph
	std::vector<long long> edges;
	for (auto &ijl : rawEdges) {
		edges.push_back(make_fast_triple(v_map[ijl.first.first], l_map[ijl.second], v_map[ijtn.first.second]));
	}

	return std::make_tuple(std::move(v_map), std::move(l_map), std::move(edges), nv, std::vector<Grammar> {gmp, gmb});
}
