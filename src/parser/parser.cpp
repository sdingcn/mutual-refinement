#include "parser.h"
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <tuple>
#include <algorithm>
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
		std::string dtype = ijl.second.substr(1, 1);
		std::string number = ijl.second.substr(4, ijl.second.size() - 4);
		if (dtype == "p") {
			numbers["p"].insert(number);
		} else if (dtype == "b") {
			numbers["b"].insert(number);
		}
	}

#ifdef AUGMENT
	// max number of numbers considered in augmentation
	constexpr int N = 3;
	std::string meta_number = "meta_202111021748";
	std::string close_number = "close_202111022225";
	// count the frequencies of each number
	std::map<std::string, std::map<std::string, int>> counters;
	for (auto &ijl : rawEdges) {
		std::string dtype = ijl.second.substr(1, 1);
		std::string number = ijl.second.substr(4, ijl.second.size() - 4);
		if (number == meta_number) {
			std::cerr << "Error: meta_number dup" << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (number == close_number) {
			std::cerr << "Error: close_number dup" << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (dtype == "p") {
			counters["p"][number]++;
		} else if (dtype == "b") {
			counters["b"][number]++;
		}
	}
	// put "considered" numbers into a map
	std::map<std::string, std::unordered_set<std::string>> considered;
	// find out the NP - 1 most frequent numbers, and treat other numbers as the same
	std::vector<std::pair<std::string, int>> pc;
	for (auto &p : counters["p"]) {
		pc.push_back(p);
	}
	std::sort(pc.begin(), pc.end(), [](const std::pair<std::string, int> &p1, const std::pair<std::string, int> &p2) -> bool {
		return p1.second > p2.second;
	});
	int NP = std::min(N - 1, static_cast<int>(pc.size() - 1));
	for (int i = 0; i < NP - 1; i++) {
		considered["p"].insert(pc[i].first);
	}
	// find out the NB - 1 most frequent brackets, and treat other brackets as the same
	std::vector<std::pair<std::string, int>> bc;
	for (auto &p : counters["b"]) {
		bc.push_back(p);
	}
	std::sort(bc.begin(), bc.end(), [](const std::pair<std::string, int> &p1, const std::pair<std::string, int> &p2) -> bool {
		return p1.second > p2.second;
	});
	int NB = std::min(N - 1, static_cast<int>(bc.size() - 1));
	for (int i = 0; i < NB - 1; i++) {
		considered["b"].insert(bc[i].first);
	}
	// markers for nonterminals
	std::map<std::string, std::vector<std::pair<std::string, std::string>>> markers;
	// for p
	for (auto &left : considered["b"]) {
		for (auto &right : considered["b"]) {
			markers["p"].push_back(std::make_pair(left, right));
		}
	}
	for (auto &left : considered["b"]) {
		markers["p"].push_back(std::make_pair(left, meta_number));
		markers["p"].push_back(std::make_pair(left, close_number));
	}
	for (auto &right : considered["b"]) {
		markers["p"].push_back(std::make_pair(meta_number, right));
		markers["p"].push_back(std::make_pair(close_number, right));
	}
	markers["p"].push_back(std::make_pair(meta_number, meta_number));
	markers["p"].push_back(std::make_pair(meta_number, close_number));
	markers["p"].push_back(std::make_pair(close_number, meta_number));
	markers["p"].push_back(std::make_pair(close_number, close_number));
	// for b
	for (auto &left : considered["p"]) {
		for (auto &right : considered["p"]) {
			markers["b"].push_back(std::make_pair(left, right));
		}
	}
	for (auto &left : considered["p"]) {
		markers["b"].push_back(std::make_pair(left, meta_number));
		markers["b"].push_back(std::make_pair(left, close_number));
	}
	for (auto &right : considered["p"]) {
		markers["b"].push_back(std::make_pair(meta_number, right));
		markers["b"].push_back(std::make_pair(close_number, right));
	}
	markers["b"].push_back(std::make_pair(meta_number, meta_number));
	markers["b"].push_back(std::make_pair(meta_number, close_number));
	markers["b"].push_back(std::make_pair(close_number, meta_number));
	markers["b"].push_back(std::make_pair(close_number, close_number));
#endif

#ifdef AUGMENT
	// encode labels
	std::map<label, int> l_map;
	int l_ctr = 0;
	for (auto &m : markers["p"]) {
		l_map[label{"dp", m.first, m.second}] = l_ctr++;
	}
	for (auto &n : numbers["p"]) {
		l_map[label{"op", n}] = l_ctr++;
		l_map[label{"cp", n}] = l_ctr++;
		for (auto &m : markers["p"]) {
			l_map[label{"dp", n, m.first, m.second}] = l_ctr++;
		}
	}
	for (auto &m : markers["b"]) {
		l_map[label{"db", m.first, m.second}] = l_ctr++;
	}
	for (auto &n : numbers["b"]) {
		l_map[label{"ob", n}] = l_ctr++;
		l_map[label{"cb", n}] = l_ctr++;
		for (auto &m : markers["b"]) {
			l_map[label{"db", n, m.first, m.second}] = l_ctr++;
		}
	}
#else
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
#endif

#ifdef AUGMENT
	// grammar constructor
	auto construct_grammar = [&numbers, &l_map, &meta_number, &close_number, &considered, &markers]
		(Grammar &gm, const std::string &dyck, const std::string &other_dyck) -> void {
		for (auto &n : numbers[dyck]) {
			gm.terminals.insert(l_map[label{"o" + dyck, n}]);
			gm.terminals.insert(l_map[label{"c" + dyck, n}]);
		}
		for (auto &n : numbers[other_dyck]) {
			gm.terminals.insert(l_map[label{"o" + other_dyck, n}]);
			gm.terminals.insert(l_map[label{"c" + other_dyck, n}]);
		}
		for (auto &m : markers[dyck]) {
			gm.nonterminals.insert(l_map[label{"d" + dyck, m.first, m.second}]);
		}
		for (auto &m : markers[dyck]) {
			for (auto &n : numbers[dyck]) {
				gm.nonterminals.insert(l_map[label{"d" + dyck, n, m.first, m.second}]);
			}
		}
		// d      -> empty
		for (auto &m : markers[dyck]) {
			gm.emptyProductions.push_back(l_map[label{"d" + dyck, m.first, m.second}]);
		}
		// d      -> d d
		for (auto &m1 : markers[dyck]) {
			for (auto &m2 : markers[dyck]) {
				if (m1.second == close_number || m2.first == close_number || m1.second == m2.first) {
					gm.binaryProductions.push_back(std::make_pair(
								l_map[label{"d" + dyck, m1.first, m2.second}],
								std::make_pair(
									l_map[label{"d" + dyck, m1.first, m1.second}],
									l_map[label{"d" + dyck, m2.first, m2.second}]
									)
								));
				}
			}
		}
		// d      -> o--[n] d--[n]
		// d--[n] -> d      c--[n]
		for (auto &m : markers[dyck]) {
			for (auto &n : numbers[dyck]) {
				gm.binaryProductions.push_back(std::make_pair(
							l_map[label{"d" + dyck, m.first, m.second}],
							std::make_pair(
								l_map[label{"o" + dyck, n}],
								l_map[label{"d" + dyck, n, m.first, m.second}]
								)
							));
				gm.binaryProductions.push_back(std::make_pair(
							l_map[label{"d" + dyck, n, m.first, m.second}],
							std::make_pair(
								l_map[label{"d" + dyck, m.first, m.second}],
								l_map[label{"c" + dyck, n}])
							));
			}
		}
		// d      -> ...
		for (auto &n : numbers[other_dyck]) {
			if (considered[other_dyck].count(n) > 0) {
				gm.unaryProductions.push_back(std::make_pair(
							l_map[label{"d" + dyck, close_number, n}],
							l_map[label{"o" + other_dyck, n}]
							));
				gm.unaryProductions.push_back(std::make_pair(
							l_map[label{"d" + dyck, n, close_number}],
							l_map[label{"c" + other_dyck, n}]
							));
			} else {
				gm.unaryProductions.push_back(std::make_pair(
							l_map[label{"d" + dyck, close_number, meta_number}],
							l_map[label{"o" + other_dyck, n}]
							));
				gm.unaryProductions.push_back(std::make_pair(
							l_map[label{"d" + dyck, meta_number, close_number}],
							l_map[label{"c" + other_dyck, n}]
							));
			}
		}
		gm.startSymbol = l_map[label{"d" + dyck, close_number, close_number}];
		gm.init(l_map.size());
	};
#else
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
#endif

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

#ifdef VERBOSE
	std::cerr << "v_size: " << v_map.size() << ", l_size: " << l_map.size() << std::endl;
#endif

	return std::make_tuple(std::move(v_map), std::move(l_map), std::move(edges), std::vector<Grammar> {gmp, gmb});
}
