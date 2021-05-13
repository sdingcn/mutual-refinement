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
using std::ifstream;
using std::getline;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stoi;
using std::reverse;
using std::max;

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

	// edges
	vector<set<pair<int, int>>> fastEdgeTest; // first vertex -> {(label, second vertex)}
	vector<vector<pair<int, int>>> adjacencyVector; // first vertex -> [(label, second vertex)]
	vector<vector<pair<int, int>>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]

	// records for reachability closures
	vector<map<int, set<int>>> unaryRecord; // i -> j -> {unary production number}
	vector<map<int, set<pair<int, int>>>> binaryRecord; // i -> j -> {(binary prodution number, middle vertex)}

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

	bool hasEdge(int i, int x, int j) {
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
				// if (!hasEdge(i, x, i)) {
				addEdge(i, x, i);
				w.push(make_pair(make_pair(i, i), x));
				// }
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
					if (!hasEdge(i, x, j)) {
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
							if (!hasEdge(i, x, k)) {
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
							if (!hasEdge(k, x, j)) {
								addEdge(k, x, j);
								w.push(make_pair(make_pair(k, j), x));
							}
						}
					}
				}
			}
		}
	}

	set<int> getCFLReachabilityClosure(int i, int j, const Grammar &g) {
		if (!hasEdge(i, g.startSymbol, j)) {
			return set<int>();
		} else {
			set<int> ret {i, j};
			using State = pair<pair<int, int>, int>; // ((i, j), x) i --x--> j
			State start = make_pair(make_pair(i, j), g.startSymbol);
			set<State> vis;
			queue<State> q;
			vis.insert(start);
			q.push(start);
			while (!q.empty()) { // BFS
				State cur = q.front();
				q.pop();

				// i --x--> j
				int i = cur.first.first;
				int j = cur.first.second;
				int x = cur.second;

				for (int ind : unaryRecord[i][j]) {
					if (g.unaryProductions[ind].first == x) {
						State nxt = make_pair(make_pair(i, j), g.unaryProductions[ind].second);
						if (vis.count(nxt) == 0) {
							vis.insert(nxt);
							q.push(nxt);
						}
					}
				}
				for (auto ind_k : binaryRecord[i][j]) {
					int ind = ind_k.first;
					int k = ind_k.second; // i --> k --> j
					if (g.binaryProductions[ind].first == x) {
						ret.insert(k);
						State nxt1 = make_pair(make_pair(i, k), g.binaryProductions[ind].second.first);
						State nxt2 = make_pair(make_pair(k, j), g.binaryProductions[ind].second.second);
						if (vis.count(nxt1) == 0) {
							vis.insert(nxt1);
							q.push(nxt1);
						}
						if (vis.count(nxt2) == 0) {
							vis.insert(nxt2);
							q.push(nxt2);
						}
					}
				}
			}
			return ret;
		}
	}
};

void test() {
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
				assert(gh.hasEdge(i, gm.startSymbol, j));
			} else {
				assert(!gh.hasEdge(i, gm.startSymbol, j));
			}
		}
	}
	auto rc1 = gh.getCFLReachabilityClosure(0, 2, gm);
	assert(rc1.size() == 5);
	assert(rc1.count(0) == 1);
	assert(rc1.count(1) == 1);
	assert(rc1.count(2) == 1);
	assert(rc1.count(4) == 1);
	assert(rc1.count(5) == 1);
	auto rc2 = gh.getCFLReachabilityClosure(2, 3, gm);
	assert(rc2.size() == 2);
	assert(rc2.count(2) == 1);
	assert(rc2.count(3) == 1);
}

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

// This function is expected to return the normalized data.
//     That means the vertices should be 0, 1, ..., n - 1.
vector<pair<pair<int, int>, int>> getEdges(string fname) {
	ifstream in(fname); // automatically closed after leaving this function
	string line;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			auto p = parseLine(line);
			cout << p.first.first << ' ' << p.first.second << ' ' << p.second << endl;
		}
	}
	return vector<pair<pair<int, int>, int>>();
}

vector<Grammar> getGrammars(string fname) {
	return vector<Grammar>();
}

pair<int, int> getSourceAndSink(string fname) {
	pair<int, int> ret(0, 0);
	return ret;
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		test();
		return 0;
	} else {
		auto edges = getEdges(argv[1]);
		auto grammars = getGrammars(argv[1]);
		auto ss = getSourceAndSink(argv[1]);
		int source = ss.first;
		int sink = ss.second;
		int n = 0;
		for (auto ijs : edges) { // finding out the number of vertices in the original graph.
			n = max(n, ijs.first.first);
			n = max(n, ijs.first.second);
		}
		++n;
		int rounds = grammars.size(); // the number of CFLs that we want to intersect
		set<int> vertices; // The vertices that we want to consider.
		for (int i = 0; i < n; i++) { // Initially, we consider all vertices.
			vertices.insert(i);
		}
		for (int r = 0; r < rounds; r++) {
			Graph gh(n);
			for (auto ijs : edges) { // We only include edges containing vertices that we want to consider.
				if (vertices.count(ijs.first.first) == 1 &&
				    vertices.count(ijs.first.second) == 1) {
					gh.addEdge(ijs.first.first, ijs.second, ijs.first.second);
				}
			}
			gh.runCFLReachability(grammars[r]);
			vertices = gh.getCFLReachabilityClosure(source, sink, grammars[r]);
			if (vertices.size() == 0) {
				cout << "Definitely Unreachable" << endl;
				return 0;
			}
		}
		cout << "Possibly Reachable" << endl;
		return 0;
	}
}
