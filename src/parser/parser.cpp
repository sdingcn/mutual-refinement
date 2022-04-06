#include "parser.h"
#include "../hasher/hasher.h"
#include "../error/error.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <utility>

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

bool isSymbol(const std::string &s) {
	return isNonterminal(s) || isTerminal(s);
}

bool isNode(const std::string &s) {
	int sz = s.size();
	if (sz == 0) {
		return false;
	}
	for (int i = 0; i < sz; i++) {
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
	int sz = lines.size();
	if (sz == 0) {
		return false;
	}
	if (!isNonterminal(lines[0])) {
		return false;
	}
	for (int i = 1; i < sz; i++) {
		std::istringstream sin(lines[i]);
		std::vector<std::string> elements;
		std::string elem;
		while (sin >> elem) {
			elements.push_back(elem);
		}
		if (elements.size() == 1) {
			if (!isNonterminal(elements[0])) {
				return false;
			}
		} else if (elements.size() == 2) {
			if ((!isNonterminal(elements[0])) ||
			    (!isSymbol(elements[1]))) {
				return false;
			}
		} else if (elements.size() == 3) {
			if ((!isNonterminal(elements[0])) ||
			    (!isSymbol(elements[1])) ||
			    (!isSymbol(elements[2]))) {
				return false;
			}
		} else {
			return false;
		}
	}
	return true;
}

Grammar parseGrammar(const std::string &fname, std::unordered_map<std::string, int> &sym_map) {
	// file auto closed via destructor
	std::ifstream in(fname);
	if (in.fail()) {
		error(38764588);
	}
	// read all lines and check
	std::vector<std::string> lines;
	std::string line;
	while (getline(in, line)) {
		lines.push_back(line);
	}
	if (!checkGrammar(lines)) {
		error(12327939);
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

std::pair<std::pair<std::string, std::string>, std::string> parseGraphLine(const std::string &line) {
	// node1->node2[label="label"]
	std::string::size_type p1 = line.find("->");
	std::string::size_type p2 = line.find('[');
	std::string::size_type p3 = line.find('=');
	std::string::size_type p4 = line.find(']');
	return std::make_pair(std::make_pair(line.substr(0, p1 - 0), line.substr(p1 + 2, p2 - (p1 + 2))),
			      line.substr(p3 + 2, p4 - 1 - (p3 + 2)));
}

bool checkGraph(const std::vector<std::string> &lines) {
	// check graph file's grammar and number boundaries
	int sz = lines.size();
	for (int i = 0; i < sz; i++) {
		// node1->node2[label="label"]
		std::string::size_type arrow_pos = lines[i].find("->");
		if (arrow_pos == std::string::npos) {
			return false;
		}
		std::string::size_type label_open_pos = lines[i].find("[label=\"");
		if (label_open_pos == std::string::npos) {
			return false;
		}
		std::string::size_type label_close_pos = lines[i].find("\"]");
		if (label_close_pos == std::string::npos) {
			return false;
		}
		if (!(arrow_pos + 2 < label_open_pos &&
		      label_open_pos + 8 < label_close_pos &&
		      label_close_pos + 2 == lines[i].size())) {
			return false;
		}
		std::string node1 = lines[i].substr(0, arrow_pos - 0);
		std::string node2 = lines[i].substr(arrow_pos + 2, label_open_pos - (arrow_pos + 2));
		std::string label = lines[i].substr(label_open_pos + 8, label_close_pos - (label_open_pos + 8));
		if (!isNode(node1)) {
			return false;
		}
		if (!isNode(node2)) {
			return false;
		}
		if (!isLabel(label)) {
			return false;
		}
	}
	return true;
}

std::pair<int, std::unordered_set<std::tuple<int, int, int>, IntTripleHasher>> parseGraph(const std::string &fname,
		const std::unordered_map<std::string, int> &sym_map,
		std::unordered_map<std::string, int> &node_map) {
	// file auto closed via destructor
	std::ifstream in(fname);
	if (in.fail()) {
		error(28723488);
	}
	// read all lines and check
	std::vector<std::string> lines;
	std::string line;
	while (getline(in, line)) {
		lines.push_back(line);
	}
	if (!checkGraph(lines)) {
		error(91327939);
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
	// constructing graph
	int n = lines.size();
	std::unordered_set<std::tuple<int, int, int>, IntTripleHasher> edges;
	for (int i = 0; i < n; i++) {
		auto l = parseGraphLine(lines[i]);
		edges.insert(std::make_tuple(convert(l.first.first), sym_map.at(l.second), convert(l.first.second)));
	}
	int nv = node_map.size();
	return std::make_pair(nv, std::move(edges));
}

std::vector<Grammar> extractDyck(const std::string &fname,
		std::unordered_map<std::string, int> &sym_map) {
	// the label type
	using label_type = std::vector<std::string>;

	// file auto closed via destructor
	std::ifstream in(fname);
	if (in.fail()) {
		error(23450237);
	}

	// read raw lines
	std::vector<std::string> lines;
	std::string line;
	while (getline(in, line)) {
		lines.push_back(line);
	}
	if (!checkGraph(lines)) {
		error(92334739);
	}

	// get all labels
	std::vector<std::string> labels;
	for (auto line : lines) {
		labels.push_back(parseGraphLine(line).second);
	}

	// find out numbers for each Dyck
	std::unordered_map<std::string, std::unordered_set<std::string>> numbers;
	for (auto &label : labels) {
		// cp--10
		std::string dtype = label.substr(1, 1);
		std::string number = label.substr(4, label.size() - 4);
		numbers[dtype].insert(number);
	}

	// encode labels
	std::map<label_type, int> label_map;
	int label_num = 0;
	/*
	 * d -> empty
	 * d -> d d
	 * d -> o* d1
	 * d1 -> d c*
	 */
	label_map[label_type{"d"}] = label_num++;
	label_map[label_type{"d1"}] = label_num++;
	/*
	 * dp      -> empty
	 * dp      -> dp dp
	 * dp      -> op--[n] dp--[n]
	 * dp--[n] -> dp cp--[n]
	 * dp      -> ob* | cb*
	 */
	label_map[label_type{"dp"}] = label_num++;
	for (auto &n : numbers["p"]) {
		label_map[label_type{"op", n}] = label_num++;
		label_map[label_type{"cp", n}] = label_num++;
		label_map[label_type{"dp", n}] = label_num++;
	}
	/*
	 * db      -> empty
	 * db      -> db db
	 * db      -> ob--[n] db--[n]
	 * db--[n] -> db cb--[n]
	 * db      -> op* | cp*
	 */
	label_map[label_type{"db"}] = label_num++;
	for (auto &n : numbers["b"]) {
		label_map[label_type{"ob", n}] = label_num++;
		label_map[label_type{"cb", n}] = label_num++;
		label_map[label_type{"db", n}] = label_num++;
	}

	// fill sym_map
	auto join_str = [](const std::vector<std::string> &label, const std::string &joiner) -> std::string {
		int n = label.size();
		std::string ret;
		if (n > 0) {
			ret += label[0];
			for (int i = 1; i < n; i++) {
				ret += joiner;
				ret += label[i];
			}
		}
		return ret;
	};
	for (auto &p : label_map) {
		sym_map[join_str(p.first, "--")] = p.second;
	}

	// grammar constructors
	auto construct_combined = [&numbers, &label_map]
		(Grammar &gm) -> void {
		for (auto &n : numbers["p"]) {
			gm.addTerminal(label_map[label_type{"op", n}]);
			gm.addTerminal(label_map[label_type{"cp", n}]);
		}
		for (auto &n : numbers["b"]) {
			gm.addTerminal(label_map[label_type{"ob", n}]);
			gm.addTerminal(label_map[label_type{"cb", n}]);
		}
		gm.addNonterminal(label_map[label_type{"d"}]);
		gm.addNonterminal(label_map[label_type{"d1"}]);
		// d      -> empty
		gm.addEmptyProduction(label_map[label_type{"d"}]);
		// d      -> d d
		gm.addBinaryProduction(
				label_map[label_type{"d"}],
				label_map[label_type{"d"}],
				label_map[label_type{"d"}]);
		// d      -> o d1
		// d1     -> d c
		for (auto &dyck : std::vector<std::string>{"p", "b"}) {
			for (auto &n : numbers[dyck]) {
				gm.addBinaryProduction(
						label_map[label_type{"d"}],
						label_map[label_type{"o" + dyck, n}],
						label_map[label_type{"d1"}]);
				gm.addBinaryProduction(
						label_map[label_type{"d1"}],
						label_map[label_type{"d"}],
						label_map[label_type{"c" + dyck, n}]);
			}
		}
		gm.addStartSymbol(label_map[label_type{"d"}]);
		gm.initFastIndices();
	};
	auto construct_single = [&numbers, &label_map]
		(Grammar &gm, const std::string &dyck, const std::string &other_dyck) -> void {
		for (auto &n : numbers[dyck]) {
			gm.addTerminal(label_map[label_type{"o" + dyck, n}]);
			gm.addTerminal(label_map[label_type{"c" + dyck, n}]);
		}
		for (auto &n : numbers[other_dyck]) {
			gm.addTerminal(label_map[label_type{"o" + other_dyck, n}]);
			gm.addTerminal(label_map[label_type{"c" + other_dyck, n}]);
		}
		gm.addNonterminal(label_map[label_type{"d" + dyck}]);
		for (auto &n : numbers[dyck]) {
			gm.addNonterminal(label_map[label_type{"d" + dyck, n}]);
		}
		// d      -> empty
		gm.addEmptyProduction(label_map[label_type{"d" + dyck}]);
		// d      -> d d
		gm.addBinaryProduction(
				label_map[label_type{"d" + dyck}],
				label_map[label_type{"d" + dyck}],
				label_map[label_type{"d" + dyck}]);
		// d      -> o--[n] d--[n]
		// d--[n] -> d      c--[n]
		for (auto &n : numbers[dyck]) {
			gm.addBinaryProduction(
					label_map[label_type{"d" + dyck}],
					label_map[label_type{"o" + dyck, n}],
					label_map[label_type{"d" + dyck, n}]);
			gm.addBinaryProduction(
					label_map[label_type{"d" + dyck, n}],
					label_map[label_type{"d" + dyck}],
					label_map[label_type{"c" + dyck, n}]);
		}
		// d      -> ...
		for (auto &n : numbers[other_dyck]) {
			gm.addUnaryProduction(
					label_map[label_type{"d" + dyck}],
					label_map[label_type{"o" + other_dyck, n}]);
			gm.addUnaryProduction(
					label_map[label_type{"d" + dyck}],
					label_map[label_type{"c" + other_dyck, n}]);
		}
		gm.addStartSymbol(label_map[label_type{"d" + dyck}]);
		gm.initFastIndices();
	};

	// construct grammars
	Grammar gmc, gmp, gmb;
	construct_combined(gmc);
	construct_single(gmp, "p", "b");
	construct_single(gmb, "b", "p");

	return std::vector<Grammar> {gmp, gmc, gmb};
}
