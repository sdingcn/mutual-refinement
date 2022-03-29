#ifndef PARSER_H
#define PARSER_H

#include "../common.h"
#include "../grammar/grammar.h"
#include "../graph/graph.h"
#include <string>
#include <vector>
#ifdef ROBIN
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
template <typename K, typename V> using hash_map = tsl::robin_map<K, V>;
template <typename T> using hash_set = tsl::robin_set<T>;
#else
#include <unordered_map>
#include <unordered_set>
template <typename K, typename V> using hash_map = std::unordered_map<K, V>;
template <typename T> using hash_set = std::unordered_set<T>;
#endif
#include <utility>

Grammar parseGrammar(const std::string &fname, hash_map<std::string, int> &sym_map);

std::pair<int, hash_set<long long>> parseGraph(const std::string &fname,
		const hash_map<std::string, int> &sym_map,
		hash_map<std::string, int> &node_map);

std::vector<Grammar> extractDyck(const std::string &fname,
		hash_map<std::string, int> &sym_map);

#endif
