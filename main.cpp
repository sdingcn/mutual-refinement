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

	// This requires that startSymbol could be inserted into arbitrary positions arbitrarily many times in valid strings.
	// For a typical grammar of the Dyck language (or the unmatched variant), this is the case.
	// forall s in negligibleTerminals, startSymbol -> s
	// Netligible terminals are not included in terminals. They are not present in the productions either.
	set<int> negligibleTerminals;
};

struct Graph {
	int numberOfVertices;

	// The index of the outmost vector serves as the first vertex.
	vector<set<pair<int, int>>> fastEdgeTest; // (label, second vertex)
	vector<vector<pair<int, int>>> adjacencyVector; // (label, second vertex)
	vector<vector<pair<int, int>>> counterAdjacencyVector; // (first vertex, label)

	// records for reachability closures
	vector<map<int, set<int>>> unaryRecord; // [i][j][productionNumber]
	vector<map<int, set<pair<int, int>>>> binaryRecord; // [i][j][(productionNumber, middleVertex)]

	Graph(int n) : numberOfVertices(n),
                       fastEdgeTest(n),
                       adjacencyVector(n),
                       counterAdjacencyVector(n),
                       unaryRecord(n),
                       binaryRecord(n) {}

	void addEdge(int i, int x, int j) { // i --x--> j
		fastEdgeTest[i].insert(make_pair(x, j));
		adjacencyVector[i].push_back(make_pair(x, j));
		counterAdjacencyVector[j].push_back(make_pair(i, x));
	}

	bool isEdge(int i, int x, int j) {
		return fastEdgeTest[i].count(make_pair(x, j)) == 1;
	}

	void runCFLReachability(const Grammar &g) {
		queue<pair<pair<int, int>, int>> w; // ((first vertex, second vertex), label)
		vector<pair<int, int>> negligibleEdges;
		for (int i = 0; i < numberOfVertices; i++) { // add all edges to the worklist, and find out all negligible edges
			for (auto &sj : adjacencyVector[i]) { // --s--> j
				w.push(make_pair(make_pair(i, sj.second), sj.first));
				if (g.negligibleTerminals.count(sj.first) == 1) {
					negligibleEdges.push_back(make_pair(i, sj.second));
				}
			}
		}
		for (auto e : negligibleEdges) { // add negligible edges to the edge set and the worklist
			addEdge(e.first, g.startSymbol, e.second);
			w.push(make_pair(make_pair(e.first, e.second), g.startSymbol));
		}
		int nep = g.emptyProductions.size();
		int nup = g.unaryProductions.size();
		int nbp = g.binaryProductions.size();
		for (int ind = 0; ind < nep; ind++) { // add empty edges to the edge set and the worklist
			int x = g.emptyProductions[ind];
			for (int i = 0; i < numberOfVertices; i++) {
				if (!isEdge(i, x, i)) {
					addEdge(i, x, i);
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
					unaryRecord[i][j].insert(ind);
					if (!isEdge(i, x, j)) {
						addEdge(i, x, j);
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
							binaryRecord[i][k].insert(make_pair(ind, j));
							if (!isEdge(i, x, k)) {
								addEdge(i, x, k);
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
							binaryRecord[k][j].insert(make_pair(ind, i));
							if (!isEdge(k, x, j)) {
								addEdge(k, x, j);
								w.push(make_pair(make_pair(k, j), x));
							}
						}
					}
				}
			}
		}
	}

	set<int> getReachabilityClosure(int i, int j, const Grammar &g) {
		if (!isEdge(i, g.startSymbol, j)) {
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
	 *
	 * 10 is a negligible terminal. We have 0 -> 10.
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
	gm.negligibleTerminals.insert(10);
	/*
	 *  1->(4)-10->(5)-2->
	 *  |                 \    -1->
	 *  |                  \  | /
	 * (0) --1--> (1) --2--> (2) --2--> (3)
	 *  |                     |
	 *   ----------1--------->
	 */
	Graph gh(6);
	gh.addEdge(0, 1, 4);
	gh.addEdge(4, 10, 5);
	gh.addEdge(5, 2, 2);
	gh.addEdge(0, 1, 1);
	gh.addEdge(1, 2, 2);
	gh.addEdge(2, 1, 2);
	gh.addEdge(2, 2, 3);
	gh.addEdge(0, 1, 2);
	gh.runCFLReachability(gm);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			if ((i == j) ||
			    (i == 0 && j == 2) ||
			    (i == 0 && j == 3) ||
			    (i == 2 && j == 3) ||
			    (i == 4 && j == 5)) {
				assert(gh.isEdge(i, gm.startSymbol, j));
			} else {
				assert(!gh.isEdge(i, gm.startSymbol, j));
			}
		}
	}
	auto rc1 = gh.getReachabilityClosure(0, 2, gm);
	assert(rc1.size() == 5);
	assert(rc1.count(0) == 1);
	assert(rc1.count(1) == 1);
	assert(rc1.count(2) == 1);
	assert(rc1.count(4) == 1);
	assert(rc1.count(5) == 1);
	auto rc2 = gh.getReachabilityClosure(2, 3, gm);
	assert(rc2.size() == 2);
	assert(rc2.count(2) == 1);
	assert(rc2.count(3) == 1);
}
