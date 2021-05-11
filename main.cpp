#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <queue>
#include <cassert>
#include <iostream>

using std::pair;
using std::make_pair;
using std::vector;
using std::set;
using std::queue;
using std::cerr;
using std::endl;

struct Grammar {
	set<int> terminals;
	set<int> nonterminals;
	vector<int> emptyProductions;
	vector<pair<int, int>> unaryProductions;
	vector<pair<int, pair<int, int>>> binaryProductions;
	int startSymbol;
};

struct Graph {
	int numberOfVertices;

	// The index of the outmost vector serves as the first vertex.
	vector<vector<pair<int, int>>> adjacencyVector; // (label, second vertex)
	vector<vector<pair<int, int>>> counterAdjacencyVector; // (first vertex, label)
	vector<set<pair<int, int>>> fastMembershipTest; // (label, second vertex)

	Graph(int n) : numberOfVertices(n),
                       adjacencyVector(n),
		       counterAdjacencyVector(n),
	               fastMembershipTest(n) {}

	void addEdge(int i, int x, int j) { // i --x--> j
		auto xj = make_pair(x, j);
		fastMembershipTest[i].insert(xj);
		adjacencyVector[i].push_back(xj);
		counterAdjacencyVector[j].push_back(make_pair(i, x));
	}

	void runCFLReachability(const Grammar &g) {
		queue<pair<pair<int, int>, int>> w; // ((first vertex, second vertex), label)
		for (int i = 0; i < numberOfVertices; i++) { // add all edges to the worklist
			for (auto &sj : adjacencyVector[i]) { // --s--> j
				w.push(make_pair(make_pair(i, sj.second), sj.first));
			}
		}
		for (int x : g.emptyProductions) { // add empty edges to the edge set and the worklist
			for (int i = 0; i < numberOfVertices; i++) {
				auto xi = make_pair(x, i); // --x--> i
				if (fastMembershipTest[i].count(xi) == 0) {
					fastMembershipTest[i].insert(xi);
					adjacencyVector[i].push_back(xi);
					counterAdjacencyVector[i].push_back(make_pair(i, x));
					w.push(make_pair(make_pair(i, i), x));
				}
			}
		}
		while (!w.empty()) {
			auto e = w.front();
			w.pop();

			// i --y--> j
			int i = e.first.first;
			int j = e.first.second;
			int y = e.second;

			for (auto &p : g.unaryProductions) {
				if (p.second == y) { // x -> y
					int x = p.first;
					auto xj = make_pair(x, j); // --x--> j
					if (fastMembershipTest[i].count(xj) == 0) {
						fastMembershipTest[i].insert(xj);
						adjacencyVector[i].push_back(xj);
						counterAdjacencyVector[j].push_back(make_pair(i, x));
						w.push(make_pair(make_pair(i, j), x));
					}
				}
			}
			for (auto &p : g.binaryProductions) {
				if (p.second.first == y) { // x -> yz
					int x = p.first;
					int z = p.second.second;
					for (auto &sk : adjacencyVector[j]) { // --s--> k
						if (sk.first == z) { // --z--> k
							int k = sk.second;
							auto xk = make_pair(x, k);
							if (fastMembershipTest[i].count(xk) == 0) {
								fastMembershipTest[i].insert(xk);
								adjacencyVector[i].push_back(xk);
								counterAdjacencyVector[k].push_back(make_pair(i, x));
								w.push(make_pair(make_pair(i, k), x)); 
							}
						}
					}
				}
				if (p.second.second == y) { // x -> zy
					int x = p.first;
					int z = p.second.first;
					for (auto &ks : counterAdjacencyVector[i]) {
						if (ks.second == z) { // k --z-->
							int k = ks.first;
							auto xj = make_pair(x, j);
							if (fastMembershipTest[k].count(xj) == 0) {
								fastMembershipTest[k].insert(xj);
								adjacencyVector[k].push_back(xj);
								counterAdjacencyVector[j].push_back(make_pair(k, x));
								w.push(make_pair(make_pair(k, j), x));
							}
						}
					}
				}
			}
		}
	}
};

int main()
{
	/*
	 * D[0] -> D[0] D[0] | ([1] Dc[3] | epsilon
	 * Dc[3] -> D[0] )[2]
	 *
	 * 0 -> 0 0 | 1 3 | epsilon
	 * 3 -> 0 2
	 */
	Grammar gm;
	gm.terminals.insert(1);
	gm.terminals.insert(2);
	gm.nonterminals.insert(0);
	gm.nonterminals.insert(3);
	gm.emptyProductions.push_back(0);
	gm.binaryProductions.push_back(make_pair(0, make_pair(0, 0)));
	gm.binaryProductions.push_back(make_pair(0, make_pair(1, 3)));
	gm.binaryProductions.push_back(make_pair(3, make_pair(0, 2)));
	gm.startSymbol = 0;
	/*
	 * (0) --1--> (1) --2--> (2) --2--> (3)
	 *  |                     |
	 *   ----------1--------->
	 */
	Graph gh(4);
	gh.addEdge(0, 1, 1);
	gh.addEdge(1, 2, 2);
	gh.addEdge(2, 2, 3);
	gh.addEdge(0, 1, 2);
	gh.runCFLReachability(gm);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if ((i == j) ||
			    (i == 0 && j == 2) ||
			    (i == 0 && j == 3)) {
				assert(gh.fastMembershipTest[i].count(make_pair(gm.startSymbol, j)) == 1);
			} else {
				assert(gh.fastMembershipTest[i].count(make_pair(gm.startSymbol, j)) == 0);
			}
		}
	}
}
