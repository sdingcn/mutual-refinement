#include "parser.h"
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include "../common.h"
#include "../die.h"

bool isUpperLetter(char c) {
	static const char *upper_letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for (int i = 0; i < 26; i++) {
		if (c == upper_letters[i]) {
			return true;
		}
	}
	return false;
}

bool isLowerLetter(char c) {
	static const char *lower_letters = "abcdefghijklmnopqrstuvwxyz";
	for (int i = 0; i < 26; i++) {
		if (c == lower_letters[i]) {
			return true;
		}
	}
	return false;
}

bool isDigit(char c) {
	static const char *digits = "0123456789";
	for (int i = 0; i < 10; i++) {
		if (c == digits[i]) {
			return true;
		}
	}
	return false;
}

bool isDash(char c) {
	return c == '-' || c == '_';
}

bool isNonterminal(const std::string &s) {
	int sz = s.size();
	if (sz == 0) {
		return false;
	}
	if (!isUpperLetter(s[0])) {
		return false;
	}
	for (int i = 1; i < sz; i++) {
		if (!(isUpperLetter(s[i]) || isLowerLetter(s[i]) || isDigit(s[i]) || isDash(s[i]))) {
			return false;
		}
	}
	return true;
}

bool isTerminal(const std::string &s) {
	int sz = s.size();
	if (sz == 0) {
		return false;
	}
	if (!isLowerLetter(s[0])) {
		return false;
	}
	for (int i = 1; i < sz; i++) {
		if (!(isUpperLetter(s[i]) || isLowerLetter(s[i]) || isDigit(s[i]) || isDash(s[i]))) {
			return false;
		}
	}
	return true;
}

bool isNode(const std::string &s) {
	int sz = s.size();
	if (sz == 0) {
		return false;
	}
	for (int i = 1; i < sz; i++) {
		if (!(isUpperLetter(s[i]) || isLowerLetter(s[i]) || isDigit(s[i]) || isDash(s[i]))) {
			return false;
		}
	}
	return true;
}

bool isLabel(const std::string &s) {
	return isTerminal(s);
}

bool checkGrammar(const std::vector<std::string> &lines) {
	return true;
}

Grammar parseGrammar(const std::string &fname, std::unordered_map<std::string, int> &sym_map) {
	// file auto closed via destructor
	std::ifstream in(fname);
	if (in.fail()) {
		die(38764588);
	}
	// read all lines and check
	std::vector<std::string> lines;
	std::string line;
	while (getline(in, line)) {
		lines.push_back(line);
	}
	if (!checkGrammar(lines)) {
		die(12327939);
	}
	// converter
	int num = 0;
	auto convert = [&num, &sym_map](const std::string &sym) -> int {
		if (sym_map.count(sym) == 1) {
			return sym_map[sym];
		} else {
			return sym_map[sym] = num++;
		}
	};
	// constructing grammar
	Grammar grammar;
	int start = convert(lines[0]);
	grammar.addNonterminal(start);
	grammar.addStartSymbol(start);
	int n = lines.size();
	for (int i = 1; i < n; i++) {
		std::istringstream sin(lines[i]);
		std::vector<std::string> words;
		std::string word;
		while (sin >> word) {
			words.push_back(word);
			if (isTerminal(word)) {
				grammar.addTerminal(convert(word));
			} else {
				grammar.addNonterminal(convert(word));
			}
		}
		if (words.size() == 1) {
			grammar.addEmptyProduction(convert(words[0]));
		} else if (words.size() == 2) {
			grammar.addUnaryProduction(convert(words[0]), convert(words[1]));
		} else if (words.size() == 3) {
			grammar.addBinaryProduction(convert(words[0]), convert(words[1]), convert(words[2]));
		}
	}
	grammar.initFastIndices();
	return grammar;
}

bool checkGraph(const std::vector<std::string> &lines) {
	return true;
}

std::pair<std::pair<std::string, std::string>, std::string> parseGraphLine(const std::string &line) {
	// node1->node2[label="label1"]
	std::string::size_type p1 = line.find("->");
	std::string::size_type p2 = line.find('[');
	std::string::size_type p3 = line.find('=');
	std::string::size_type p4 = line.find(']');
	return std::make_pair(
			std::make_pair(line.substr(0, p1 - 0), line.substr(p1 + 2, p2 - (p1 + 2))),
			line.substr(p3 + 2, p4 - 1 - (p3 + 2))
			);
}

std::pair<int, std::unordered_set<long long>> parseGraph(const std::string &fname,
		const std::unordered_map<std::string, int> &sym_map,
		std::unordered_map<std::string, int> &node_map) {
	// file auto closed via destructor
	std::ifstream in(fname);
	if (in.fail()) {
		die(28723488);
	}
	// read all lines and check
	std::vector<std::string> lines;
	std::string line;
	while (getline(in, line)) {
		lines.push_back(line);
	}
	if (!checkGraph(lines)) {
		die(91327939);
	}
	// converter
	int num = 0;
	auto convert = [&num, &node_map](const std::string &node) -> int {
		if (node_map.count(node) == 1) {
			return node_map[node];
		} else {
			return node_map[node] = num++;
		}
	};
	// count the number of vertices
	int n = lines.size();
	for (int i = 0; i < n; i++) {
		auto l = parseGraphLine(lines[i]);
		convert(l.first.first);
		convert(l.first.second);
	}
	int nv = node_map.size();
	// constructing graph
	std::unordered_set<long long> edges;
	for (int i = 0; i < n; i++) {
		auto l = parseGraphLine(lines[i]);
		edges.insert(make_fast_triple(convert(l.first.first), sym_map.at(l.second), convert(l.first.second)));
	}
	return std::make_pair(nv, std::move(edges));
}
