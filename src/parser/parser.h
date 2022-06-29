#ifndef PARSER_H
#define PARSER_H

#include "../hasher/hasher.h"
#include "../grammar/grammar.h"
#include "../graph/graph.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

struct GraphFile {
	std::unordered_map<std::string, int> nodeMap;
	std::unordered_map<int, std::string> nodeMapR;
	std::unordered_map<std::string, int> symMap;
	std::unordered_map<int, std::string> symMapR;
	std::vector<Grammar> grammars;
	std::unordered_set<std::tuple<int, int, int>, IntTripleHasher> edges;
};

GraphFile parseGraphFile(const std::string &fName);

#endif
