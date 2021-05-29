#ifndef COMMON_H
#define COMMON_H

#include <utility>
#include <tuple>

using Edge = std::tuple<int, int, int>; // (first vertex, label, second vertex)

constexpr long long WIDTH = 20LL;
constexpr long long MASK = (1LL << WIDTH) - 1LL;

inline long long make_fast_pair(int a, int b) { // assuming a >= 0 && b >= 0
	return (static_cast<long long>(a) << WIDTH) + static_cast<long long>(b);
}

inline long long pack_fast_pair(const std::pair<int, int> &p) {
	return make_fast_pair(p.first, p.second);
}

inline std::pair<int, int> unpack_fast_pair(long long fp) {
	return std::make_pair(static_cast<int>(fp >> WIDTH), static_cast<int>(fp & MASK));
}

inline long long make_fast_triple(int a, int b, int c) { // assuming a >= 0 && b >= 0 && c >= 0
	return (static_cast<long long>(a) << (WIDTH << 1LL))
		+ (static_cast<long long>(b) << WIDTH)
		+ static_cast<long long>(c);
}

inline long long pack_fast_triple(const std::tuple<int, int, int> &t) {
	return make_fast_triple(std::get<0>(t), std::get<1>(t), std::get<2>(t));
}

inline std::tuple<int, int, int> unpack_fast_triple(long long ft) {
	return std::make_tuple(
			static_cast<int>(ft >> (WIDTH << 1LL)),
			static_cast<int>((ft >> WIDTH) & MASK),
			static_cast<int>(ft & MASK));
}

#endif
