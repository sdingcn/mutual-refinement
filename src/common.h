#ifndef COMMON_H
#define COMMON_H

constexpr long long WIDTH = 20LL;
constexpr long long MASK = (1LL << WIDTH) - 1LL;

inline long long make_fast_pair(int a, int b) {
	return (static_cast<long long>(a) << WIDTH) | static_cast<long long>(b);
}

inline int fast_pair_first(long long fp) {
	return static_cast<int>(fp >> WIDTH);
}

inline int fast_pair_second(long long fp) {
	return static_cast<int>(fp & MASK);
}

inline long long make_fast_triple(int a, int b, int c) {
	return (static_cast<long long>(a) << (WIDTH << 1LL))
		| (static_cast<long long>(b) << WIDTH)
		| static_cast<long long>(c);
}

inline long long make_fast_triple(int a, long long bc) {
	return (static_cast<long long>(a) << (WIDTH << 1LL))
		| static_cast<long long>(bc);
}

inline int fast_triple_first(long long ft) {
	return static_cast<int>(ft >> (WIDTH << 1LL));
}

inline int fast_triple_second(long long ft) {
	return static_cast<int>((ft >> WIDTH) & MASK);
}

inline int fast_triple_third(long long ft) {
	return static_cast<int>(ft & MASK);
}

inline long long fast_triple_tail(long long ft) {
	return static_cast<long long>(ft & (MASK | (MASK << WIDTH)));
}

#endif
