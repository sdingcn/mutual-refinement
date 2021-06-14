#ifndef PARSER_H
#define PARSER_H

#include <tuple>
#include <utility>
#include <vector>
#include <map>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"

// v_map (original vertex name -> number), edges, number of vertices, grammars
std::tuple<std::map<std::string, int>, std::vector<long long>, int, std::vector<Grammar>> parsePAGraph(const std::string &fname);

std::tuple<std::vector<long long>, int, std::pair<int, int>, std::vector<Grammar>> parseBPGraph(const std::string &fname);

#endif
