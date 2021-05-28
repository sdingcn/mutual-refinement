#ifndef PARSER_H
#define PARSER_H

#include <tuple>
#include <vector>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"

std::tuple<std::vector<Edge>, int, std::vector<Grammar>> parsePAGraph(const std::string &fname);

#endif
