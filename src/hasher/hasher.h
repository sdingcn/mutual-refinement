#ifndef HASHER_H
#define HASHER_H

#include <cstddef>
#include <utility>
#include <tuple>

struct IntPairHasher {
	std::size_t operator () (const std::pair<int, int> &p) const;
};

struct IntTripleHasher {
	std::size_t operator () (const std::tuple<int, int, int> &t) const;
};

using Edge = std::tuple<int, int, int>;
using EdgeHasher = IntTripleHasher;

#endif
