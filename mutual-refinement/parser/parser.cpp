#include "parser.h"
#include <string>
#include <utility>
#include <algorithm>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <tuple>
#include <cassert>
#include "../common.h"

// common functions

// line -> contains(line, '>')
bool isEdgeLine(const std::string &line) {
	for (char c : line) {
		if (c == '>') {
			return true;
		}
	}
	return false;
}

// start number, original names -> v_map (original name -> number)
std::map<std::string, int> normalize(int start, const std::vector<std::string> &ss) {
	std::map<std::string, int> m;
	for (auto &s : ss) {
		m[s] = 0;
	}
	int i = start;
	for (auto &pr : m) {
		pr.second = i++;
	}
	return m;
}

// line -> ((vertex1, vertex2), (label1, label2))
std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>> parsePALine(std::string line) {
	std::string::size_type p1, p2, p3, v1pos, v1len, v2pos, v2len, l1pos, l1len, l2pos, l2len;
	p1 = line.find("->");
	p2 = line.find('[');
	p3 = line.find(']');
	v1pos = 0;
	v1len = p1 - v1pos;
	v2pos = p1 + 2;
	v2len = p2 - v2pos;
	l1pos = p2 + 8;
	l1len = 2;
	l2pos = p2 + 12;
	l2len = p3 - 1 - l2pos;
	return std::make_pair(
			std::make_pair(line.substr(v1pos, v1len), line.substr(v2pos, v2len)),
			std::make_pair(line.substr(l1pos, l1len), line.substr(l2pos, l2len))
			);
}

// fname -> (v_map (original vertex name -> number), edges, number of vertices, grammars)
std::tuple<std::map<std::string, int>, std::map<std::string, int>, std::vector<long long>, int, std::vector<Grammar>>
parsePAGraph(const std::string &fname) {
	std::ifstream in(fname); // file auto closed via destructor

	// read raw edges
	std::string line;
	std::vector<std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>>> rawEdges;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			rawEdges.push_back(parsePALine(line));
		}
	}

	// normalize vertices
	// 0, 1, ..., n - 1
	std::vector<std::string> v;
	for (auto &ijtn : rawEdges) {
		v.push_back(ijtn.first.first);
		v.push_back(ijtn.first.second);
	}
	auto v_map = normalize(0, v);
	int nv = v_map.size();

	// normalize labels
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2
	std::vector<std::string> d1;
	std::vector<std::string> d2;
	for (auto &ijtn : rawEdges) {
		if (ijtn.second.first == "op" || ijtn.second.first == "cp") {
			d1.push_back(ijtn.second.second);
		} else {
			d2.push_back(ijtn.second.second);
		}
	}
	// d_i_map : original parenthesis index (string) -> the number of the corresponding open parenthesis
	auto d1_map = normalize(0, d1);
	int nd1 = d1_map.size();
	auto d2_map = normalize(2 * nd1, d2);
	int nd2 = d2_map.size();

	// Once we have the numbers of labels, we can proceed and construct the grammar using those numbers.
	// symbols (universal among all grammars)
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]    [ first Dyck's nonterminals    ]    [ second Dyck's nonterminals   ]
	// (_1, ... (_nd1, )_1, ... )_nd1    [_1, ... [_nd2, ]_1, ... ]_nd2    D, D_1, D_2, D_3, D_4, ... D_nd1    E, E_1, E_2, E_3, E_4, ... E_nd2
	Grammar gm1, gm2;
	auto fillGrammar = [](Grammar &gm, int t_start, int nd, int other_t_start, int other_nd, int nt_start) -> void {
		for (int i = t_start; i < t_start + 2 * nd; i++) {
			gm.terminals.insert(i);
		}
		for (int i = other_t_start; i < other_t_start + 2 * other_nd; i++) {
			gm.terminals.insert(i);
		}
		for (int i = nt_start; i <= nt_start + nd; i++) {
			gm.nonterminals.insert(i);
		}
		// D -> empty
		gm.emptyProductions.push_back(nt_start);
		// D -> D D
		gm.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(nt_start, nt_start)));
		for (int i = 0; i < nd; i++) {
			// D -> (_i D_i
			gm.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(t_start + i, nt_start + 1 + i)));
			// D_i -> D )_i
			gm.binaryProductions.push_back(std::make_pair(nt_start + 1 + i, std::make_pair(nt_start, t_start + nd + i)));
		}
		for (int i = other_t_start; i < other_t_start + 2 * other_nd; i++) {
			// D -> ...
			gm.unaryProductions.push_back(std::make_pair(nt_start, i));
		}
		gm.startSymbol = nt_start;
	};
	fillGrammar(gm1,       0, nd1, 2 * nd1, nd2, 2 * nd1 + 2 * nd2);
	fillGrammar(gm2, 2 * nd1, nd2,       0, nd1, 2 * nd1 + 2 * nd2 + nd1 + 1);
	int total = 2 * nd1 + 2 * nd2 + nd1 + 1 + nd2 + 1;
	gm1.fillInv(total);
	gm2.fillInv(total);

	// check boundaries
	if (nv - 1 > static_cast<int>(MASK)) {
		std::cerr << "Error: The graph contains too many nodes." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	if (total - 1 > static_cast<int>(MASK)) {
		std::cerr << "Error: The grammar contains too many symbols." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// construct the normalized graph
	std::vector<long long> edges;
	std::map<std::string, int> l_map;
	for (auto &ijtn : rawEdges) {
		std::string &t = ijtn.second.first;
		std::string &n = ijtn.second.second;
		int sym;
		if (t == "op") {
			sym = d1_map[n];
		} else if (t == "cp") {
			sym = d1_map[n] + nd1;
		} else if (t == "ob") {
			sym = d2_map[n];
		} else {
			sym = d2_map[n] + nd2;
		}
		l_map[t + "--" + n] = sym;
		edges.push_back(make_fast_triple(v_map[ijtn.first.first], sym, v_map[ijtn.first.second]));
	}

	return std::make_tuple(std::move(v_map), std::move(l_map), std::move(edges), nv, std::vector<Grammar> {gm1, gm2});
}
