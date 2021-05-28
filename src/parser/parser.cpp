#include "parser.h"
#include <string>
#include <utility>
#include <algorithm>
#include <vector>
#include <map>
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

std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>> parseLine(std::string line) {
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

std::tuple<std::vector<Edge>, int, std::vector<Grammar>> parsePAGraph(const std::string &fname) {
	std::ifstream in(fname); // file auto closed via destructor

	// read raw edges
	std::string line;
	std::vector<std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>>> rawEdges;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			rawEdges.push_back(parseLine(line));
		}
	}

	// normalize vertices
	// 0, 1, ..., n - 1
	std::vector<std::string> v;
	for (auto &ijtn : rawEdges) {
		v.push_back(ijtn.first.first);
		v.push_back(ijtn.first.second);
	}
	auto nv_map = normalize(0, v);
	int n = nv_map.size();

	// normalize labels
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2
	std::vector<std::string> p;
	std::vector<std::string> b;
	for (auto &ijtn : rawEdges) {
		if (ijtn.second.first == "op" || ijtn.second.first == "cp") {
			p.push_back(ijtn.second.second);
		} else {
			b.push_back(ijtn.second.second);
		}
	}
	auto np_map = normalize(0, p);
	int n1 = np_map.size();
	auto nb_map = normalize(2 * n1, b);
	int n2 = nb_map.size();

	// Once we have the numbers of labels, we can proceed and construct the grammar using those numbers.
	// symbols (universal among all grammars)
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]    [ first Dyck's nonterminals    ]    [ second Dyck's nonterminals   ]
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2    D, D_1, D_2, D_3, D_4, ..., D_n1    E, E_1, E_2, E_3, E_4, ..., E_n2
	Grammar gmp, gmb;
	auto fillGrammar = [](Grammar &gm, int op1, int n1, int op2, int n2, int start) -> void {
		for (int i = op1; i < op1 + 2 * n1; i++) {
			gm.terminals.insert(i);
		}
		for (int i = op2; i < op2 + 2 * n2; i++) {
			gm.terminals.insert(i);
		}
		for (int i = start; i <= start + n1; i++) {
			gm.nonterminals.insert(i);
		}
		// D -> empty
		gm.emptyProductions.push_back(start);
		// D -> D D
		gm.binaryProductions.push_back(std::make_pair(start, std::make_pair(start, start)));
		for (int i = 0; i < n1; i++) {
			// D -> (_i D_i
			gm.binaryProductions.push_back(std::make_pair(start, std::make_pair(op1 + i, start + 1 + i)));
			// D_i -> D )_i
			gm.binaryProductions.push_back(std::make_pair(start + 1 + i, std::make_pair(start, op1 + n1 + i)));
		}
		for (int i = op2; i < op2 + 2 * n2; i++) {
			// D -> [_i
			// D -> ]_i
			gm.unaryProductions.push_back(std::make_pair(start, i));
		}
		gm.startSymbol = start;
		for (int i = op2; i < op2 + n2; i++) {
			gm.leftToRight[i] = i + n2;
		}
		for (int i = op2 + n2; i < op2 + 2 * n2; i++) {
			gm.rightToLeft[i] = i - n2;
		}
	};
	fillGrammar(gmp, 0, n1, 2 * n1, n2, 2 * n1 + 2 * n2);
	fillGrammar(gmb, 2 * n1, n2, 0, n1, 2 * n1 + 2 * n2 + n1 + 1);
	int total = 2 * n1 + 2 * n2 + n1 + 1 + n2 + 1;
	gmp.fillInv(total);
	gmb.fillInv(total);

	// check boundaries
	if (n > FP_MASK) {
		std::cerr << "Error: The graph contains too many nodes." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	if (total > FP_MASK) {
		std::cerr << "Error: The grammar contains too many symbols." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// construct the normalized graph
	std::vector<Edge> edges;
	for (auto &ijtn : rawEdges) {
		std::string &t = ijtn.second.first;
		std::string &n = ijtn.second.second;
		int sym;
		if (t == "op") {
			sym = np_map[n];
		} else if (t == "cp") {
			sym = np_map[n] + n1;
		} else if (t == "ob") {
			sym = nb_map[n];
		} else {
			sym = nb_map[n] + n2;
		}
		edges.push_back(std::make_tuple(nv_map[ijtn.first.first], sym, nv_map[ijtn.first.second]));
	}

	return std::make_tuple(edges, n, std::vector<Grammar> {gmp, gmb});
}
