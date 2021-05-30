#ifndef PARSER_H
#define PARSER_H

#include <tuple>
#include <utility>
#include <vector>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"

std::tuple<std::vector<long long>, int, std::vector<Grammar>> parsePAGraph(const std::string &fname);
std::tuple<std::vector<long long>, int, std::pair<int, int>, std::vector<Grammar>> parseBPGraph(const std::string &fname);

#endif
