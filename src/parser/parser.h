#ifndef PARSER_H
#define PARSER_H

#include <tuple>
#include <vector>
#include <map>
#include <unordered_set>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"
#include "../graph/graph.h"

Grammar parseGrammar(const std::string &fname, std::unordered_map<std::string, int> &sym_map);

std::pair<int, std::unordered_set<long long>> parseGraph(const std::string &fname,
		const std::unordered_map<std::string, int> &sym_map,
		std::unordered_map<std::string, int> &node_map);

#endif
