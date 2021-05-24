#include "parser.h"
#include <string>
#include <utility>
#include <algorithm>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include "../common.h"

bool isEdgeLine(const std::string &line) {
	for (char c : line) {
		if (c == '>') {
			return true;
		}
	}
	return false;
}

std::pair<std::pair<int, int>, std::pair<std::string, int>> parseLine(std::string line) {
	int v1, v2;
	std::string label;
	std::reverse(line.begin(), line.end());
	std::string buffer;
	while (true) {
		if (line.back() == '-') {
			v1 = stoi(buffer);
			buffer = "";
			line.pop_back();
			line.pop_back();
			break;
		}
		buffer.push_back(line.back());
		line.pop_back();
	}
	while (true) {
		if (line.back() == '[') {
			v2 = stoi(buffer);
			buffer = "";
			for (int i = 0; i < 8; i++) {
				line.pop_back();
			}
			break;
		}
		buffer.push_back(line.back());
		line.pop_back();
	}
	while (true) {
		if (line.back() == '"') {
			label = buffer;
			break;
		}
		buffer.push_back(line.back());
		line.pop_back();
	}
	return std::make_pair(std::make_pair(v1, v2), std::make_pair(label.substr(0, 2), stoi(label.substr(4))));
}

std::map<int, int> normalizeNumbers(int start, const std::vector<int> &numbers) {
	std::map<int, int> m;
	for (int n : numbers) {
		m[n] = 0;
	}
	int i = start;
	for (auto &pr : m) {
		pr.second = i++;
	}
	return m;
}

std::pair<std::pair<std::vector<Edge>, std::pair<int, int>>, std::vector<Grammar>> parseFile(const std::string &fname) {
	std::ifstream in(fname); // file auto closed via destructor

	// read raw edges
	std::string line;
	std::vector<std::pair<std::pair<int, int>, std::pair<std::string, int>>> rawEdges;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			rawEdges.push_back(parseLine(line));
		}
	}

	// vertices
	// 0, 1, ..., n - 1

	// normalize vertices
	std::vector<int> v;
	for (auto &ijtn : rawEdges) {
		v.push_back(ijtn.first.first);
		v.push_back(ijtn.first.second);
	}
	auto nv_map = normalizeNumbers(0, v);
	int n = nv_map.size();

	// symbols
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]    [ first Dyck's nonterminals    ]    [ second Dyck's nonterminals   ]
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2    D, D_1, D_2, D_3, D_4, ..., D_n1    E, E_1, E_2, E_3, E_4, ..., E_n2

	// normalize labels
	std::vector<int> p;
	std::vector<int> b;
	for (auto &ijtn : rawEdges) {
		if (ijtn.second.first == "op" || ijtn.second.first == "cp") {
			p.push_back(ijtn.second.second);
		} else {
			b.push_back(ijtn.second.second);
		}
	}
	auto np_map = normalizeNumbers(0, p);
	int n1 = np_map.size();
	auto nb_map = normalizeNumbers(2 * n1, b);
	int n2 = nb_map.size();

	Grammar gmp, gmb;
	auto fillDyck = [](Grammar &gm, int op_begin, int n, int start) -> void {
		for (int i = op_begin; i < op_begin + 2 * n; i++) {
			gm.terminals.insert(i);
		}
		for (int i = start; i <= start + n; i++) {
			gm.nonterminals.insert(i);
		}
		gm.emptyProductions.push_back(start);
		gm.binaryProductions.push_back(std::make_pair(start, std::make_pair(start, start)));
		for (int i = 0; i < n; i++) {
			gm.binaryProductions.push_back(std::make_pair(start, std::make_pair(op_begin + i, start + 1 + i)));
			gm.binaryProductions.push_back(std::make_pair(start + 1 + i, std::make_pair(start, op_begin + n + i)));
		}
		gm.startSymbol = start;
	};
	fillDyck(gmp, 0, n1, 2 * n1 + 2 * n2);
	for (int i = 2 * n1; i < 2 * n1 + 2 * n2; i++) {
		gmp.terminals.insert(i);
		gmp.unaryProductions.push_back(std::make_pair(2 * n1 + 2 * n2, i));
	}
	fillDyck(gmb, 2 * n1, n2, 2 * n1 + 2 * n2 + n1 + 1);
	for (int i = 0; i < 2 * n1; i++) {
		gmb.terminals.insert(i);
		gmb.unaryProductions.push_back(std::make_pair(2 * n1 + 2 * n2 + n1 + 1, i));
	}
	int total = 2 * n1 + 2 * n2 + n1 + 1 + n2 + 1;
	gmp.fillInv(total);
	gmb.fillInv(total);

	if (n > FP_MASK) {
		std::cerr << "Error: The graph contains too many nodes." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	if (total > FP_MASK) {
		std::cerr << "Error: The grammar contains too many symbols." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	std::vector<Edge> edges;
	for (auto &ijtn : rawEdges) {
		std::string &t = ijtn.second.first;
		int n = ijtn.second.second;
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
		edges.push_back(make_edge(nv_map[ijtn.first.first], sym, nv_map[ijtn.first.second]));
	}

	return std::make_pair(std::make_pair(edges, std::make_pair(0, n - 1)), std::vector<Grammar> {gmp, gmb});
}
