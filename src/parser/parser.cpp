#include "parser.h"
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <tuple>
#include "../common.h"

bool isEdgeLine(const std::string &line) {
	for (char c : line) {
		if (c == '>') {
			return true;
		}
	}
	return false;
}

std::pair<std::pair<std::string, std::string>, std::string> parsePALine(const std::string &line) {
	// 100->200[label="cp--10"]
	std::string::size_type p1 = line.find("->");
	std::string::size_type p2 = line.find('[');
	std::string::size_type p3 = line.find(']');
	return std::make_pair(
			std::make_pair(line.substr(0, p1 - 0), line.substr(p1 + 2, p2 - (p1 + 2))),
			line.substr(p2 + 8, p3 - 1 - (p2 + 8))
			);
}

std::tuple<
	std::map<std::string, int>,
	std::map<std::vector<std::string>, int>,
	std::vector<long long>,
	std::vector<Grammar>
> parsePAGraph(const std::string &fname) {
	// the label type
	using label = std::vector<std::string>;

	// file auto closed via destructor
	std::ifstream in(fname);
	if (in.fail()) {
		std::cerr << "Error: Cannot open the file" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// read raw edges
	std::string line;
	std::vector<std::pair<std::pair<std::string, std::string>, std::string>> rawEdges;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			rawEdges.push_back(parsePALine(line));
		}
	}

	// encode vertices
	std::map<std::string, int> v_map;
	int v_ctr = 0;
	for (auto &ijl : rawEdges) {
		auto i = ijl.first.first;
		auto j = ijl.first.second;
		if (v_map.count(i) == 0) {
			v_map[i] = v_ctr++;
		}
		if (v_map.count(j) == 0) {
			v_map[j] = v_ctr++;
		}
	}

	// find out numbers for each Dyck
	std::map<std::string, std::unordered_set<std::string>> numbers;
	for (auto &ijl : rawEdges) {
		if (ijl.second.substr(0, 2) == "op" || ijl.second.substr(0, 2) == "cp") {
			numbers["p"].insert(ijl.second.substr(4, ijl.second.size() - 4));
		}
		if (ijl.second.substr(0, 2) == "ob" || ijl.second.substr(0, 2) == "cb") {
			numbers["b"].insert(ijl.second.substr(4, ijl.second.size() - 4));
		}
	}

	// encode labels
	std::map<label, int> l_map;
	int l_ctr = 0;
	l_map[label{"dp"}] = l_ctr++;
	for (auto &n : numbers["p"]) {
		l_map[label{"op", n}] = l_ctr++;
		l_map[label{"cp", n}] = l_ctr++;
		l_map[label{"dp", n}] = l_ctr++;
	}
	l_map[label{"db"}] = l_ctr++;
	for (auto &n : numbers["b"]) {
		l_map[label{"ob", n}] = l_ctr++;
		l_map[label{"cb", n}] = l_ctr++;
		l_map[label{"db", n}] = l_ctr++;
	}

	// grammar constructor
	auto construct_grammar = [&numbers, &l_map](Grammar &gm, const std::string &dyck, const std::string &other_dyck) -> void {
		for (auto &n : numbers[dyck]) {
			gm.terminals.insert(l_map[label{"o" + dyck, n}]);
			gm.terminals.insert(l_map[label{"c" + dyck, n}]);
		}
		for (auto &n : numbers[other_dyck]) {
			gm.terminals.insert(l_map[label{"o" + other_dyck, n}]);
			gm.terminals.insert(l_map[label{"c" + other_dyck, n}]);
		}
		gm.nonterminals.insert(l_map[label{"d" + dyck}]);
		for (auto &n : numbers[dyck]) {
			gm.nonterminals.insert(l_map[label{"d" + dyck, n}]);
		}
		// d      -> empty
		gm.emptyProductions.push_back(l_map[label{"d" + dyck}]);
		// d      -> d d
		gm.binaryProductions.push_back(std::make_pair(
					l_map[label{"d" + dyck}],
					std::make_pair(
						l_map[label{"d" + dyck}],
						l_map[label{"d" + dyck}]
						)
					));
		// d      -> o--[n] d--[n]
		// d--[n] -> d      c--[n]
		for (auto &n : numbers[dyck]) {
			gm.binaryProductions.push_back(std::make_pair(
						l_map[label{"d" + dyck}],
						std::make_pair(
							l_map[label{"o" + dyck, n}],
							l_map[label{"d" + dyck, n}]
							)
						));
			gm.binaryProductions.push_back(std::make_pair(
						l_map[label{"d" + dyck, n}],
						std::make_pair(
							l_map[label{"d" + dyck}],
							l_map[label{"c" + dyck, n}])
						));
		}
		// d      -> ...
		for (auto &n : numbers[other_dyck]) {
			gm.unaryProductions.push_back(std::make_pair(
						l_map[label{"d" + dyck}],
						l_map[label{"o" + other_dyck, n}]
						));
			gm.unaryProductions.push_back(std::make_pair(
						l_map[label{"d" + dyck}],
						l_map[label{"c" + other_dyck, n}]
						));
		}
		gm.startSymbol = l_map[label{"d" + dyck}];
		gm.init(l_map.size());
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
		edges.push_back(make_fast_triple(
					v_map[ijl.first.first],
					l_map[label{ijl.second.substr(0, 2), ijl.second.substr(4, ijl.second.size() - 4)}],
					v_map[ijl.first.second]
					));
	}

	return std::make_tuple(std::move(v_map), std::move(l_map), std::move(edges), std::vector<Grammar> {gmp, gmb});
}
