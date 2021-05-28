#ifndef COMMON_H
#define COMMON_H

#include <utility>
#include <tuple>

using Edge = std::tuple<int, int, int>; // (first vertex, label, second vertex)

constexpr long long FP_MASK = (1LL << 30);

inline long long make_fast_pair(int a, int b) { // assuming a >= 0 && b >= 0
	return a * FP_MASK + b;
}

inline std::pair<int, int> unpack_fast_pair(long long fp) {
	return std::make_pair(static_cast<int>(fp / FP_MASK), static_cast<int>(fp % FP_MASK));
}

#endif
