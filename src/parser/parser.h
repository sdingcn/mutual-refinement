#ifndef PARSER_H
#define PARSER_H

#include <tuple>
#include <vector>
#include <unordered_map>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"

// v_map, l_map, edges, grammars
std::tuple<
	std::unordered_map<std::string, int>,
	std::unordered_map<std::string, int>,
	std::vector<long long>,
	std::vector<Grammar>
> parsePAGraph(const std::string &fname);

#endif
