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

	// normalize vertices
	// 0, 1, ..., n - 1
	std::vector<int> v;
	for (auto &ijtn : rawEdges) {
		v.push_back(ijtn.first.first);
		v.push_back(ijtn.first.second);
	}
	auto nv_map = normalizeNumbers(0, v);
	int n = nv_map.size();

	// normalize labels
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2
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

	// Once we have the numbers of labels, we can proceed and construct the grammar.
#if 0
	// symbols (universal among all grammars)
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2
	// [ first Dyck's nonterminals                         ]    [ second Dyck's nonterminals                        ]
	// D[(n2 + 1)^2], D_1[(n2 + 1)^2], ..., D_n1[(n2 + 1)^2]    E[(n1 + 1)^2], E_1[(n1 + 1)^2], ..., E_n2[(n1 + 1)^2]
	Grammar gmp, gmb;
	auto fillGrammar = [](Grammar &gm, int op1, int n1, int op2, int n2, int start) -> void {
		int step = (n2 + 1) * (n2 + 1);
		for (int i = op1; i < op1 + 2 * n1; i++) {
			gm.terminals.insert(i);
		}
		for (int i = op2; i < op2 + 2 * n2; i++) {
			gm.terminals.insert(i);
		}
		for (int i = start; i < start + (n1 + 1) * step; i++) {
			gm.nonterminals.insert(i);
		}
		// ind in [0, n1]
		// indl in [0, n2]
		// indr in [0, n2]
		auto constructNonterminal = [&](int ind, int indl, int indr) -> int {
			return start + ind * step + indl * (n2 + 1) + indr;
		};
		// D[0, 0] -> empty
		// gm.emptyProductions.push_back(constructNonterminal(0, 0, 0));
		// D -> D D
		auto iterAdd = [&](int i, int j) -> void {
			gm.binaryProductions.push_back(std::make_pair(
						constructNonterminal(0, i, j),
						std::make_pair(constructNonterminal(0, i, 0), constructNonterminal(0, 0, j))));
			for (int k = 1; k <= n2; k++) {
				gm.binaryProductions.push_back(std::make_pair(
							constructNonterminal(0, i, j),
							std::make_pair(constructNonterminal(0, i, 0), constructNonterminal(0, k, j))));
				gm.binaryProductions.push_back(std::make_pair(
							constructNonterminal(0, i, j),
							std::make_pair(constructNonterminal(0, i, k), constructNonterminal(0, 0, j))));
				gm.binaryProductions.push_back(std::make_pair(
							constructNonterminal(0, i, j),
							std::make_pair(constructNonterminal(0, i, k), constructNonterminal(0, k, j))));
			}
		};
		// D[0, 0] -> D[0, 0] D[0, 0] | D[0, 0] D[i, 0] | D[0, i] D[0, 0] | D[0, i] D[i, 0]
		iterAdd(0, 0);
		for (int i = 1; i <= n2; i++) {
			// D[0, i] -> D[0, 0] D[0, i] | D[0, 0] D[j, i] | D[0, j] D[0, i] | D[0, j] D[j, i]
			iterAdd(0, i);
			// D[i, 0] -> D[i, 0] D[0, 0] | D[i, 0] D[j, 0] | D[i, j] D[0, 0] | D[i, j] D[j, 0]
			iterAdd(i, 0);
		}
		for (int i = 1; i <= n2; i++) {
			for (int j = 1; j <= n2; j++) {
				// D[i, j] -> D[i, 0] D[0, j] | D[i, 0] D[k, j] | D[i, k] D[0, j] | D[i, k] D[k, j]
				iterAdd(i, j);
			}
		}
		for (int indl = 0; indl <= n2; indl++) {
			for (int indr = 0; indr <= n2; indr++) {
				for (int i = 1; i <= n1; i++) {
					// D[indl, indr] -> (_i D_i[indl, indr]
					gm.binaryProductions.push_back(std::make_pair(
								constructNonterminal(0, indl, indr),
								std::make_pair(op1 + i - 1, constructNonterminal(i, indl, indr))
								));
					// D_i[indl, indr] -> D[indl, indr] )_i
					gm.binaryProductions.push_back(std::make_pair(
								constructNonterminal(i, indl, indr),
								std::make_pair(constructNonterminal(0, indl, indr), op1 + n1 + i - 1)
								));
				}
			}
		}
		for (int i = 1; i <= n2; i++) {
			// D[0, i] -> [_i
			gm.unaryProductions.push_back(std::make_pair(constructNonterminal(0, 0, i), op2 + i - 1));
			// D[i, 0] -> ]_i
			gm.unaryProductions.push_back(std::make_pair(constructNonterminal(0, i, 0), op2 + n2 + i - 1));
		}
		gm.startSymbol = constructNonterminal(0, 0, 0);
	};
	fillGrammar(gmp, 0, n1, 2 * n1, n2, 2 * n1 + 2 * n2);
	fillGrammar(gmb, 2 * n1, n2, 0, n1, 2 * n1 + 2 * n2 + (n1 + 1) * (n2 + 1) * (n2 + 1));
	int total = 2 * n1 + 2 * n2 + (n1 + 1) * (n2 + 1) * (n2 + 1) + (n2 + 1) * (n1 + 1) * (n1 + 1);
	gmp.fillInv(total);
	gmb.fillInv(total);
#else
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
#endif

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
