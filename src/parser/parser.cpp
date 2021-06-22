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

	// ***** Up to this point, everything is shared among the grammars. *****

	// Once we have the numbers of labels, we can proceed and construct the grammar using those numbers.

#define AMBIGUOUS_GRAMMAR

#ifdef UNAMBIGUOUS_GRAMMAR
	// symbols (universal among all grammars)
	// [ first Dyck's terminals   ]    [ second Dyck's terminals  ]    [ first Dyck's nonterminals   ]    [ second Dyck's nonterminals  ]
	// (_1 ... (_nd1, )_1 ... )_nd1    [_1 ... [_nd2, ]_1 ... ]_nd2    D, DX_1...DX_nd1, DY_1...DY_nd1    E, EX_1...EX_nd2, EY_1...E_Ynd2
	Grammar gm1, gm2;
	auto fillGrammar = [](Grammar &gm, int t_start, int nd, int other_t_start, int other_nd, int nt_start) -> void {
		for (int i = t_start; i < t_start + 2 * nd; i++) {
			gm.terminals.insert(i);
		}
		for (int i = other_t_start; i < other_t_start + 2 * other_nd; i++) {
			gm.terminals.insert(i);
		}
		for (int i = nt_start; i <= nt_start + nd * 2; i++) {
			gm.nonterminals.insert(i);
		}
		// D -> empty
		gm.emptyProductions.push_back(nt_start);
		for (int i = 0; i < nd; i++) {
			// D -> (_i DX_i
			gm.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(t_start + i, nt_start + i + 1)));
			// DX_i -> D DY_i
			gm.binaryProductions.push_back(std::make_pair(nt_start + i + 1, std::make_pair(nt_start, nt_start + nd + i + 1)));
			// DY_i -> )_i D
			gm.binaryProductions.push_back(std::make_pair(nt_start + nd + i + 1, std::make_pair(t_start + nd + i, nt_start)));
		}
		for (int i = other_t_start; i < other_t_start + 2 * other_nd; i++) {
			// D -> i D
			gm.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(i, nt_start)));
		}
		gm.startSymbol = nt_start;
	};
	fillGrammar(gm1, 0, nd1, 2 * nd1, nd2, 2 * nd1 + 2 * nd2);
	fillGrammar(gm2, 2 * nd1, nd2, 0, nd1, 2 * nd1 + 2 * nd2 + 2 * nd1 + 1);
	int total = 2 * nd1 + 2 * nd2 + 2 * nd1 + 1 + 2 * nd2 + 1;
	gm1.fillInv(total);
	gm2.fillInv(total);
#endif

#ifdef AMBIGUOUS_GRAMMAR
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
#endif

#ifdef REGULAR_AUGMENTED_GRAMMAR
	// symbols (universal among all grammars)
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]    [ first Dyck's nonterminals   ]    [ second Dyck's nonterminals  ]
	// (_1, ... (_nd1, )_1, ... )_nd1    [_1, ... [_nd2, ]_1, ... ]_nd2    D[3], D_1[3],  ...,  D_nd1[3] D    E[3], E_1[3],  ...,  E_nd2[3] E
	Grammar gm1, gm2;
	auto fillGrammar = [](Grammar &gm, int t_start, int nd, int other_t_start, int other_nd, int nt_start) -> void {
		for (int i = t_start; i < t_start + 2 * nd; i++) {
			gm.terminals.insert(i);
		}
		for (int i = other_t_start; i < other_t_start + 2 * other_nd; i++) {
			gm.terminals.insert(i);
		}
		for (int i = nt_start; i < nt_start + 3 * (nd + 1) + 1; i++) {
			gm.nonterminals.insert(i);
		}
		// D[0] -> empty
		gm.emptyProductions.push_back(nt_start);
		// D[a] -> D[0] D[a]; D[1] -> D[1] D[*]; D[2] -> D[2][*]
		gm.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(nt_start, nt_start)));
		gm.binaryProductions.push_back(std::make_pair(nt_start + 1, std::make_pair(nt_start, nt_start + 1)));
		gm.binaryProductions.push_back(std::make_pair(nt_start + 2, std::make_pair(nt_start, nt_start + 2)));

		gm.binaryProductions.push_back(std::make_pair(nt_start + 1, std::make_pair(nt_start + 1, nt_start)));
		gm.binaryProductions.push_back(std::make_pair(nt_start + 1, std::make_pair(nt_start + 1, nt_start + 1)));
		gm.binaryProductions.push_back(std::make_pair(nt_start + 1, std::make_pair(nt_start + 1, nt_start + 2)));

		gm.binaryProductions.push_back(std::make_pair(nt_start + 2, std::make_pair(nt_start + 2, nt_start)));
		gm.binaryProductions.push_back(std::make_pair(nt_start + 2, std::make_pair(nt_start + 2, nt_start + 1)));
		gm.binaryProductions.push_back(std::make_pair(nt_start + 2, std::make_pair(nt_start + 2, nt_start + 2)));

		for (int i = 0; i < nd; i++) {
			// D[a] -> (_i D_i[a]
			gm.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(t_start + i, nt_start + 3 * (1 + i))));
			gm.binaryProductions.push_back(std::make_pair(nt_start + 1, std::make_pair(t_start + i, nt_start + 3 * (1 + i) + 1)));
			gm.binaryProductions.push_back(std::make_pair(nt_start + 2, std::make_pair(t_start + i, nt_start + 3 * (1 + i) + 2)));
			// D_i[a] -> D[a] )_i
			gm.binaryProductions.push_back(std::make_pair(nt_start + 3 * (1 + i), std::make_pair(nt_start, t_start + nd + i)));
			gm.binaryProductions.push_back(std::make_pair(nt_start + 3 * (1 + i) + 1, std::make_pair(nt_start + 1, t_start + nd + i)));
			gm.binaryProductions.push_back(std::make_pair(nt_start + 3 * (1 + i) + 2, std::make_pair(nt_start + 2, t_start + nd + i)));
		}
		for (int i = other_t_start; i < other_t_start + other_nd; i++) {
			// D[1] -> [_i
			gm.unaryProductions.push_back(std::make_pair(nt_start + 1, i));
		}
		for (int i = other_t_start + other_nd; i < other_t_start + 2 * other_nd; i++) {
			// D[2] -> ]_i
			gm.unaryProductions.push_back(std::make_pair(nt_start + 2, i));
		}

		// D -> D[0]
		gm.unaryProductions.push_back(std::make_pair(nt_start + 3 * (nd + 1), nt_start));
		// D -> D[1]
		gm.unaryProductions.push_back(std::make_pair(nt_start + 3 * (nd + 1), nt_start + 1));
		gm.startSymbol = nt_start + 3 * (nd + 1);
	};
	fillGrammar(gm1,       0, nd1, 2 * nd1, nd2, 2 * nd1 + 2 * nd2);
	fillGrammar(gm2, 2 * nd1, nd2,       0, nd1, 2 * nd1 + 2 * nd2 + 3 * (nd1 + 1) + 1);
	int total = 2 * nd1 + 2 * nd2 + 3 * (nd1 + 1) + 1 + 3 * (nd2 + 1) + 1;
	gm1.fillInv(total);
	gm2.fillInv(total);
#endif

	// ********** After this point, everything is the same across the grammars. **********

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

// BP parser

std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>> parseBPLine(std::string line) {
	std::string::size_type p1, p2, p3, p4, v1pos, v1len, v2pos, v2len, l1pos, l1len, l2pos, l2len;
	p1 = line.find("->");
	p2 = line.find('[');
	p3 = line.rfind("--");
	p4 = line.rfind('"');
	v1pos = 0;
	v1len = p1 - v1pos;
	v2pos = p1 + 2;
	v2len = p2 - v2pos;
	l1pos = p2 + 8;
	l1len = p3 - l1pos;
	l2pos = p3 + 2;
	l2len = p4 - (p3 + 2);
	return std::make_pair(
			std::make_pair(line.substr(v1pos, v1len), line.substr(v2pos, v2len)),
			std::make_pair(line.substr(l1pos, l1len), line.substr(l2pos, l2len))
			);
}

std::tuple<std::vector<long long>, int, std::pair<int, int>, std::vector<Grammar>> parseBPGraph(const std::string &fname) {
	// read raw edges
	std::vector<std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>>> rawEdges;
	{
		std::ifstream in(fname); // auto close
		std::string line;
		while (getline(in, line)) {
			if (isEdgeLine(line)) {
				auto re = parseBPLine(line);
				if (re.second.second == "*") {
					auto re1 = re;
					re1.second.second = "0";
					auto re2 = re;
					re2.second.second = "1";
					rawEdges.push_back(re1);
					rawEdges.push_back(re2);
				} else {
					rawEdges.push_back(re);
				}
			}
		}
	}

	// normalize vertices
	// 0, 1, ..., nv - 1
	std::map<std::string, int> v_map;
	int nv = 0;
	{
		std::vector<std::string> v_vec;
		for (auto &ijab : rawEdges) {
			v_vec.push_back(ijab.first.first);
			v_vec.push_back(ijab.first.second);
		}
		v_map = normalize(0, v_vec);
		nv = v_map.size();
	}

	// normalize languages
	// 0, 1, ..., nl - 1
	std::map<std::string, int> l_map;
	int nl = 0;
	{
		std::vector<std::string> l_vec;
		for (auto &ijab : rawEdges) {
			// this is the label for each language
			//     for parentheses, this is just "p"
			//     for brackets, this is "b--<num>"
			l_vec.push_back(ijab.second.first.substr(1));
		}
		l_map = normalize(0, l_vec);
		nl = l_map.size();
	}

	// normalize ptypes
	std::vector<std::map<std::string, int>> ptype_maps(nl); // i -> language i's ptype label  -> language i's ptype index
	{
		std::vector<std::vector<std::string>> ptype_vecs(nl); // i -> [language i's parenthesis types]
		for (auto &ijab : rawEdges) {
			ptype_vecs[l_map[ijab.second.first.substr(1)]].push_back(ijab.second.second);
		}
		for (int i = 0; i < nl; i++) {
			ptype_maps[i] = normalize(0, ptype_vecs[i]);
		}
	}

	// total numbers of symbols
	int total = 0;
	int terminalTotal = 0;
	{
		for (int i = 0; i < nl; i++) {
			int nptype = ptype_maps[i].size();
			total += (nptype * 2 + 1 + nptype); // (_1, ..., (_nptype, )_1, ..., )_nptype, D, D_1, ..., D_nptype
			terminalTotal += (nptype * 2);
		}
	}

	// once we know the numbers of symbols we can construct the grammars
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2    ...    D, D_1, ..., D_n1    E, E_1, ..., E_n2    ...
	std::vector<Grammar> gms(nl);
	{
		auto fillGrammar = [terminalTotal](Grammar &g, int t_start, int n, int nt_start) -> void {
			for (int i = 0; i < terminalTotal; i++) {
				g.terminals.insert(i);
			}
			for (int i = nt_start; i <= nt_start + n; i++) {
				g.nonterminals.insert(i);
			}
			// D -> empty
			g.emptyProductions.push_back(nt_start);
			// D -> D D
			g.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(nt_start, nt_start)));
			for (int i = 0; i < n; i++) {
				// D -> (_i D_i
				g.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(t_start + i, nt_start + 1 + i)));
				// D_i -> D )_i
				g.binaryProductions.push_back(std::make_pair(nt_start + 1 + i, std::make_pair(nt_start, t_start + n + i)));
				// D -> (_i D
				g.binaryProductions.push_back(std::make_pair(nt_start, std::make_pair(t_start + i, nt_start)));
			}
			for (int i = 0; i < terminalTotal; i++) {
				// D -> <other terminals>
				if (i < t_start || i >= t_start + 2 * n) {
					g.unaryProductions.push_back(std::make_pair(nt_start, i));
				}
			}
			g.startSymbol = nt_start;
		};
		int t_start = 0;
		int nt_start = terminalTotal;
		for (int i = 0; i < nl; i++) {
			int nptype = ptype_maps[i].size();
			fillGrammar(gms[i], t_start, nptype, nt_start);
			gms[i].fillInv(total);
			t_start += (nptype * 2);
			nt_start += (nptype + 1);
		}
	}

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
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2    ...
	std::vector<long long> edges;
	{
		std::vector<int> prefix_sum(nl);
		{
			for (int i = 0; i < nl; i++) {
				if (i == 0) {
					prefix_sum[i] = ptype_maps[i].size() * 2;
				} else {
					prefix_sum[i] = prefix_sum[i - 1] + ptype_maps[i].size() * 2;
				}
			}
		}
		for (auto &ijab : rawEdges) {
			const std::string &a = ijab.second.first;
			const char oc = a[0];
			const std::string &lang_label = a.substr(1);
			const std::string &ptype_label = ijab.second.second;
			int lang_index = l_map[lang_label];
			int ptype_index = ptype_maps[lang_index][ptype_label];
			int sym_prefix = 0;
			if (lang_index > 0) {
				sym_prefix = prefix_sum[lang_index - 1];
			}
			int sym;
			if (oc == 'o') {
				sym = sym_prefix + ptype_index;
			} else {
				assert(oc == 'c');
				sym = sym_prefix + ptype_maps[lang_index].size() + ptype_index;
			}
			edges.push_back(make_fast_triple(v_map[ijab.first.first], sym, v_map[ijab.first.second]));
		}
	}

	return std::make_tuple(edges, nv, std::make_pair(v_map["Start"], v_map["errorentry"]), gms);
}
