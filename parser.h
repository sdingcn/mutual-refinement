#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "grammar.h"

pair<pair<vector<Edge>, pair<int, int>>, vector<Grammar>> readFile(const string &fname);

#endif
