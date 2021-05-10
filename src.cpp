#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <unordered_set>
#include <queue>

using std::pair;
using std::make_pair;
using std::vector;
using std::unordered_set;
using std::queue;

struct Grammar {
	unordered_set<int> terminals;
	unordered_set<int> nonterminals;
	vector<int> emptyProductions;
	vector<pair<int, int>> unaryProductions;
	vector<pair<int, pair<int, int>>> binaryProductions;
	int startSymbol;
};

class Graph {
	int numberOfVertices;
	vector<vector<pair<int, int>>> adjacencyVector;
	vector<unordered_set<pair<int, int>>> fastMembershipTest;
public:
	Graph(int n) : numberOfVertices(n),
                       adjacencyVector(n),
	               fastMemberShipTest(n) {}
	void addEdge(int i, int x, int j) { // i --x--> j
		adjacencyVector[i].push_back(make_pair(x, j));
	}
	void runCFLReachability(const Grammar &g) {
		queue<pair<pair<int, int>, int>> w;
		for (int i = 0; i < numberOfVertices; i++) {
			for (auto &js : adjacencyVector[i]) {
				w.push(make_pair(make_pair(i, js.first), js.second));
			}
		}
		for (int x : emptyProductions) {
			for (int v = 0; v < numberOfVertices; v++) {
				auto vx = make_pair(v, x);
				if (fastMembershipTest[v].count(vx) == 0) {
					fastMembershipTest[v].insert(vx);
					adjacencyVector[v].push_back(vx);
					w.push(make_pair(make_pair(v, v), x));
				}
			}
		}
		while (!w.empty()) {
			auto e = w.front();
			w.pop();
			int i = e.first.first;
			int j = e.first.second;
			int y = e.second;
			for (auto &p : unaryProductions) {
				if (p.second == y) { // x -> y
					int x = p.first;
					auto jx = make_pair(j, x);
					if (fastMembershipTest[i].count(jx) == 0) {
						fastMembershipTest[i].insert(jx);
						adjacencyVector[i].push_back(jx);
						w.push(make_pair(make_pair(i, j), x));
					}
				}
			}
			for (auto &p : binaryProductions) {
				if (p.second.first == y) { // x -> yz
					int x = p.first;
					int z = p.second.second;
					for (auto &ks : adjacencyVector[j]) {
						if (ks.second == z) {
							int k = ks.first;
							auto kx = make_pair(k, x);
							if (fastMembershipTest[i].count(kx) == 0) {
								fastMembershipTest[i].insert(kx);
								adjacencyVector[i].push_back(kx);
								w.push(make_pair(make_pair(i, k), 
							}
						}
					}
				}
				if (p.second.second == y) { // x -> zy
					int x = p.first;
					int z = p.second.first;
				}
			}
		}
	}
};

int main()
{
	auto graph = readGraph();
	if (graph.reachable()) {
		std::cout << "possibly reachable" << std::endl;
	} else {
		std::cout << "definitely not reachable" << std::endl;
	}
}
