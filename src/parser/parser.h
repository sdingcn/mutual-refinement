#ifndef PARSER_H
#define PARSER_H

#include <tuple>
#include <vector>
#include <map>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"

// v_map (vertex name -> number), l_map (label -> number), edges, number of vertices, grammars
std::tuple<
	std::map<std::string, int>,
	std::map<std::string, int>,
	std::vector<long long>,
	int,
	std::vector<Grammar>
> parsePAGraph(const std::string &fname);

#endif
