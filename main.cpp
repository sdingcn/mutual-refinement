#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <cassert>
#include <string>
#include <algorithm>

using std::pair;
using std::make_pair;
using std::vector;
using std::set;
using std::map;
using std::queue;
using std::cerr;
using std::endl;
using std::ifstream;
using std::getline;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stoi;
using std::reverse;

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
	// vector<map<int, set<int>>> emptyRecord; // [i][j][productionNumber]
	vector<map<int, set<int>>> unaryRecord; // [i][j][productionNumber]
	vector<map<int, set<pair<int, int>>>> binaryRecord; // [i][j][(productionNumber, middleVertex)]

	Graph(int n) : numberOfVertices(n),
                       adjacencyVector(n),
                       counterAdjacencyVector(n),
                       fastMembershipTest(n),
                       // emptyRecord(n),
                       unaryRecord(n),
                       binaryRecord(n) {}

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
		int nep = g.emptyProductions.size();
		int nup = g.unaryProductions.size();
		int nbp = g.binaryProductions.size();
		for (int ind = 0; ind < nep; ind++) { // add empty edges to the edge set and the worklist
			int x = g.emptyProductions[ind];
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

			for (int ind = 0; ind < nup; ind++) {
				auto &p = g.unaryProductions[ind];
				if (p.second == y) { // x -> y
					int x = p.first;
					auto xj = make_pair(x, j); // --x--> j
					unaryRecord[i][j].insert(ind);
					if (fastMembershipTest[i].count(xj) == 0) {
						fastMembershipTest[i].insert(xj);
						adjacencyVector[i].push_back(xj);
						counterAdjacencyVector[j].push_back(make_pair(i, x));
						w.push(make_pair(make_pair(i, j), x));
					}
				}
			}
			for (int ind = 0; ind < nbp; ind++) {
				auto &p = g.binaryProductions[ind];
				if (p.second.first == y) { // x -> yz
					int x = p.first;
					int z = p.second.second;
					for (auto &sk : adjacencyVector[j]) { // --s--> k
						if (sk.first == z) { // --z--> k
							int k = sk.second;
							auto xk = make_pair(x, k);
							binaryRecord[i][k].insert(make_pair(ind, j));
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
							binaryRecord[k][j].insert(make_pair(ind, i));
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

	set<int> getReachabilityClosure(int i, int j, const Grammar &g) {
		if (fastMembershipTest[i].count(make_pair(g.startSymbol, j)) == 0) {
			return set<int>();
		} else {
			set<int> ret;
			ret.insert(i);
			ret.insert(j);
			using State = pair<pair<int, int>, int>; // ((i, j), x) i --x--> j
			queue<State> w;
			set<State> s;
			auto start = make_pair(make_pair(i, j), g.startSymbol);
			s.insert(start);
			w.push(start);
			while (!w.empty()) {
				auto cur = w.front();
				w.pop();

				// i --x--> j
				int i = cur.first.first;
				int j = cur.first.second;
				int x = cur.second;

				for (int ind : unaryRecord[i][j]) {
					if (g.unaryProductions[ind].first == x) {
						auto nxt = make_pair(make_pair(i, j), g.unaryProductions[ind].second);
						if (s.count(nxt) == 0) {
							s.insert(nxt);
							w.push(nxt);
						}
					}
				}
				for (auto ind_k : binaryRecord[i][j]) {
					int ind = ind_k.first;
					int k = ind_k.second;
					ret.insert(k);
					if (g.binaryProductions[ind].first == x) {
						auto nxt1 = make_pair(make_pair(i, k), g.binaryProductions[ind].second.first);
						auto nxt2 = make_pair(make_pair(k, j), g.binaryProductions[ind].second.second);
						if (s.count(nxt1) == 0) {
							s.insert(nxt1);
							w.push(nxt1);
						}
						if (s.count(nxt2) == 0) {
							s.insert(nxt2);
							w.push(nxt2);
						}
					}
				}
			}
			return ret;
		}
	}
};

bool isEdgeLine(const string &line) {
	for (char c : line) {
		if (c == '>') {
			return true;
		}
	}
	return false;
}

pair<pair<int, int>, string> parseLine(string &line) {
	int v1, v2;
	string label;
	reverse(line.begin(), line.end());
	string buffer;
	while (true) {
		if (line.back() == '-') {
			v1 = stoi(buffer);
			buffer = "";
			line.pop_back();
			line.pop_back();
			break;
		}
		buffer.push_back(line.back());
		line.pop_back();
	}
	while (true) {
		if (line.back() == '[') {
			v2 = stoi(buffer);
			buffer = "";
			for (int i = 0; i < 8; i++) {
				line.pop_back();
			}
			break;
		}
		buffer.push_back(line.back());
		line.pop_back();
	}
	while (true) {
		if (line.back() == '"') {
			label = buffer;
			break;
		}
		buffer.push_back(line.back());
		line.pop_back();
	}
	return make_pair(make_pair(v1, v2), label);
}

void readGraph(string fname) {
	ifstream in(fname);
	string line;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			auto p = parseLine(line);
			cout << p.first.first << ' ' << p.first.second << ' ' << p.second << endl;
		}
	}
}

int main() {
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
	 *   --1--> (4) --2-->
	 *  |                 \    -1->
	 *  |                  \  | /
	 * (0) --1--> (1) --2--> (2) --2--> (3)
	 *  |                     |
	 *   ----------1--------->
	 */
	Graph gh(5);
	gh.addEdge(0, 1, 4);
	gh.addEdge(4, 2, 2);
	gh.addEdge(0, 1, 1);
	gh.addEdge(1, 2, 2);
	gh.addEdge(2, 1, 2);
	gh.addEdge(2, 2, 3);
	gh.addEdge(0, 1, 2);
	gh.runCFLReachability(gm);
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			if ((i == j) ||
			    (i == 0 && j == 2) ||
			    (i == 0 && j == 3) ||
			    (i == 2 && j == 3)) {
				assert(gh.fastMembershipTest[i].count(make_pair(gm.startSymbol, j)) == 1);
			} else {
				assert(gh.fastMembershipTest[i].count(make_pair(gm.startSymbol, j)) == 0);
			}
		}
	}
	auto rc1 = gh.getReachabilityClosure(0, 2, gm);
	assert(rc1.size() == 4);
	assert(rc1.count(0) == 1);
	assert(rc1.count(1) == 1);
	assert(rc1.count(2) == 1);
	assert(rc1.count(4) == 1);
	auto rc2 = gh.getReachabilityClosure(2, 3, gm);
	assert(rc2.size() == 2);
	assert(rc2.count(2) == 1);
	assert(rc2.count(3) == 1);
	readGraph("lcl-exp/taint/normal/backflash.dot");
}
