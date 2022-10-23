#ifndef PARSER_H
#define PARSER_H

#include "../hasher/hasher.h"
#include "../grammar/grammar.h"
#include "../graph/graph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// (node, label, node)
using Line = std::tuple<std::string, std::string, std::string>;

std::vector<Line> parseGraphFile(const std::string &fName);

#endif
