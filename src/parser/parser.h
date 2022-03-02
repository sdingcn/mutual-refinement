#ifndef PARSER_H
#define PARSER_H

#include <tuple>
#include <vector>
#include <map>
#include <unordered_set>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"

Grammar parseGrammar(const std::string &fname);

Grammar extractGrammarFromGraph(const std::string &fname);

std::tuple<int, std::vector<long long>> parseGraph(const std::string &fname);

#endif
