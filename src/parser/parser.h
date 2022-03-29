#ifndef PARSER_H
#define PARSER_H

#include "../common.h"
#include "../grammar/grammar.h"
#include "../graph/graph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

Grammar parseGrammar(const std::string &fname, std::unordered_map<std::string, int> &sym_map);

std::pair<int, std::unordered_set<long long>> parseGraph(const std::string &fname,
		const std::unordered_map<std::string, int> &sym_map,
		std::unordered_map<std::string, int> &node_map);

std::vector<Grammar> extractDyck(const std::string &fname,
		std::unordered_map<std::string, int> &sym_map);

#endif
