#ifndef COMMON_H
#define COMMON_H

#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <deque>
#include <cassert>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdlib>

using std::pair;
using std::make_pair;
using std::vector;
using std::set;
using std::unordered_set;
using std::map;
using std::unordered_map;
using std::deque;
using std::ifstream;
using std::getline;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stoi;
using std::reverse;
using std::max;
using std::exit;
using Edge = pair<pair<int, int>, int>; // ((first vertex, second vertex), label)

constexpr int FP_MASK = 1048576;

inline Edge make_edge(int i, int x, int j) {
	return make_pair(make_pair(i, j), x);
}

inline long long make_fast_pair(int a, int b) { // assuming a >= 0 && b >= 0
	return a * FP_MASK + b;
}

inline pair<int, int> unpack_fast_pair(long long fp) {
	return make_pair(static_cast<int>(fp / FP_MASK), static_cast<int>(fp % FP_MASK));
}

#endif
