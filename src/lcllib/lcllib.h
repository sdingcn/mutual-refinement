#ifndef LCLLIB_H
#define LCLLIB_H
#include <sstream>
#include <set>
#include <utility>
#include <string>
#include <tuple>

void runLCL(std::stringstream &graph, std::set<std::pair<std::string, std::string>> &reachablePairs,
		bool trace, std::set<std::tuple<std::string, std::string, std::string>> &contributingEdges);

#endif
