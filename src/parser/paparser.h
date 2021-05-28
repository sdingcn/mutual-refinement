#ifndef PAPARSER_H
#define PAPARSER_H

#include <utility>
#include <vector>
#include <string>
#include "../grammar/grammar.h"
#include "../common.h"

std::pair<std::pair<std::vector<Edge>, int>, std::vector<Grammar>> parsePAGraph(const std::string &fname);

#endif
