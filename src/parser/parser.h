#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <utility>
#include "../grammar/grammar.h"
#include "../common.h"
#include "../graph/graph.h"

Grammar parseGrammar(const std::string &fname, std::unordered_map<std::string, int> &sym_map);

std::pair<int, std::unordered_set<long long>> parseGraph(const std::string &fname,
		const std::unordered_map<std::string, int> &sym_map,
		std::unordered_map<std::string, int> &node_map);

std::vector<Grammar> extractDyck(const std::string &fname,
		std::unordered_map<std::string, int> &sym_map);

#endif
