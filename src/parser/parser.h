#ifndef PARSER_H
#define PARSER_H

#include <utility>
#include <vector>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"

std::pair<std::pair<std::vector<Edge>, std::pair<int, int>>, std::vector<Grammar>> parseFile(const std::string &fname);

#endif
