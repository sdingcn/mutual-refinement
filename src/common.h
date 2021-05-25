#ifndef COMMON_H
#define COMMON_H

#include <utility>

using Edge = std::pair<std::pair<int, int>, int>; // ((first vertex, second vertex), label)

constexpr long long FP_MASK = 1048576;

inline Edge make_edge(int i, int x, int j) {
	return std::make_pair(std::make_pair(i, j), x);
}

inline long long make_fast_pair(int a, int b) { // assuming a >= 0 && b >= 0
	return a * FP_MASK + b;
}

inline std::pair<int, int> unpack_fast_pair(long long fp) {
	return std::make_pair(static_cast<int>(fp / FP_MASK), static_cast<int>(fp % FP_MASK));
}

#endif
