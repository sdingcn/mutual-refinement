#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <deque>
#include <cassert>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdlib>

using std::pair;
using std::make_pair;
using std::vector;
using std::set;
using std::unordered_set;
using std::map;
using std::unordered_map;
using std::deque;
using std::ifstream;
using std::getline;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::stoi;
using std::reverse;
using std::max;
using std::exit;

struct Grammar {
	unordered_set<int> terminals;
	unordered_set<int> nonterminals;
	vector<int> emptyProductions;
	vector<pair<int, int>> unaryProductions;
	vector<pair<int, pair<int, int>>> binaryProductions;
	int startSymbol;
	vector<vector<int>> unaryProductionsInv; // right hand side symbol -> {corresponding indices in unaryProductions}
	vector<vector<int>> binaryProductionsFirstInv; // right hand side symbol 1 -> {corresponding indices in binaryProductions}
	vector<vector<int>> binaryProductionsSecondInv; // right hand side symbol 2 -> {corresponding indices in binaryProductions}
	void fillInv(int total) {
		unaryProductionsInv = vector<vector<int>>(total);
		binaryProductionsFirstInv = vector<vector<int>>(total);
		binaryProductionsSecondInv = vector<vector<int>>(total);
		int nu = unaryProductions.size();
		int nb = binaryProductions.size();
		for (int t : terminals) {
			for (int i = 0; i < nu; i++) {
				if (unaryProductions[i].second == t) {
					unaryProductionsInv[t].push_back(i);
				}
			}
			for (int i = 0; i < nb; i++) {
				if (binaryProductions[i].second.first == t) {
					binaryProductionsFirstInv[t].push_back(i);
				}
				if (binaryProductions[i].second.second == t) {
					binaryProductionsSecondInv[t].push_back(i);
				}
			}
		}
		for (int nt : nonterminals) {
			for (int i = 0; i < nu; i++) {
				if (unaryProductions[i].second == nt) {
					unaryProductionsInv[nt].push_back(i);
				}
			}
			for (int i = 0; i < nb; i++) {
				if (binaryProductions[i].second.first == nt) {
					binaryProductionsFirstInv[nt].push_back(i);
				}
				if (binaryProductions[i].second.second == nt) {
					binaryProductionsSecondInv[nt].push_back(i);
				}
			}
		}
	}
};

using Edge = pair<pair<int, int>, int>; // ((first vertex, second vertex), label)

inline Edge make_edge(int i, int x, int j) {
	return make_pair(make_pair(i, j), x);
}

#define FP_MASK 1048576

inline long long make_fast_pair(int a, int b /* assumed to be >= 0 */) {
	if (a >= 0) {
		return a * FP_MASK + b;
	} else {
		return -((-a) * FP_MASK + b);
	}
}

inline pair<int, int> unpack_fast_pair(long long fp) {
	if (fp >= 0) {
		return make_pair(static_cast<int>(fp / FP_MASK), static_cast<int>(fp % FP_MASK));
	} else {
		fp = -fp;
		return make_pair(static_cast<int>(-(fp / FP_MASK)), static_cast<int>(fp % FP_MASK));
	}
}

struct Graph {
	int numberOfVertices;

	// edges
	vector<unordered_set<long long>> fastEdgeTest; // first vertex -> {FP(label, second vertex)}
	vector<vector<pair<int, int>>> adjacencyVector; // first vertex -> [(label, second vertex)]
	vector<vector<pair<int, int>>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]

	// records for reachability closures
	vector<unordered_map<int, unordered_set<int>>> negligibleRecord; // i -> j -> {negligible symbol}
	vector<unordered_map<int, unordered_set<int>>> unaryRecord; // i -> j -> {unary production number}
	vector<unordered_map<int, unordered_set<long long>>> binaryRecord; // i -> j -> {FP(binary prodution number, middle vertex)}

	Graph(int n) : numberOfVertices(n), fastEdgeTest(n), adjacencyVector(n), counterAdjacencyVector(n),
                       negligibleRecord(n), unaryRecord(n), binaryRecord(n) {}

	void addEdge(int i, int x, int j) { // i --x--> j
		fastEdgeTest[i].insert(make_fast_pair(x, j));
		adjacencyVector[i].push_back(make_pair(x, j));
		counterAdjacencyVector[j].push_back(make_pair(i, x));
	}

	void fillEdges(const vector<Edge> &edges) {
		for (auto &ijs : edges) {
			addEdge(ijs.first.first, ijs.second, ijs.first.second);
		}
	}

	void fillEdges(const set<Edge> &edges) {
		for (auto &ijs : edges) {
			addEdge(ijs.first.first, ijs.second, ijs.first.second);
		}
	}

	bool hasEdge(int i, int x, int j) const {
		return fastEdgeTest[i].count(make_fast_pair(x, j)) == 1;
	}

	bool runPureReachability(int i, int j) {
		deque<int> q;
		unordered_set<int> s;
		q.push_back(i);
		s.insert(i);
		while (!q.empty()) {
			int c = q.front();
			q.pop_front();
			if (c == j) {
				return true;
			}
			for (auto &sj : adjacencyVector[c]) {
				if (s.count(sj.second) == 0) {
					q.push_back(sj.second);
					s.insert(sj.second);
				}
			}
		}
		return false;
	}

	void runCFLReachability(const Grammar &g) {
		// TODO: try to use the metainfo to ignore edges?
		deque<Edge> w; // ((first vertex, second vertex), label)
		vector<Edge> negligibleEdges;
		for (int i = 0; i < numberOfVertices; i++) { // add all non-negligible edges to the worklist, and find out all negligible edges
			for (auto &sj : adjacencyVector[i]) { // --s--> j
				if (g.terminals.count(sj.first) == 0) {
					negligibleEdges.push_back(make_edge(i, sj.first, sj.second));
				} else {
					w.push_back(make_edge(i, sj.first, sj.second));
				}
			}
		}
		for (auto &e : negligibleEdges) { // handle negligible edges
			negligibleRecord[e.first.first][e.first.second].insert(e.second);
			addEdge(e.first.first, g.startSymbol, e.first.second);
			w.push_back(make_edge(e.first.first, g.startSymbol, e.first.second));
		}
		int nep = g.emptyProductions.size();
		int nup = g.unaryProductions.size();
		int nbp = g.binaryProductions.size();
		for (int ind = 0; ind < nep; ind++) { // add empty edges to the edge set and the worklist
			int x = g.emptyProductions[ind];
			for (int i = 0; i < numberOfVertices; i++) {
				addEdge(i, x, i);
				w.push_back(make_edge(i, x, i));
			}
		}
		while (!w.empty()) {
			Edge e = w.front();
			w.pop_front();

			// i --y--> j
			int i = e.first.first, j = e.first.second, y = e.second;

			for (int ind : g.unaryProductionsInv[y]) { // x -> y
				auto &p = g.unaryProductions[ind];
				int x = p.first;
				unaryRecord[i][j].insert(ind);
				if (!hasEdge(i, x, j)) {
					addEdge(i, x, j);
					w.push_back(make_edge(i, x, j));
				}
			}
			for (int ind : g.binaryProductionsFirstInv[y]) { // x -> yz
				auto &p = g.binaryProductions[ind];
				int x = p.first, z = p.second.second;
				for (auto &sk : adjacencyVector[j]) { // --s--> k
					if (sk.first == z) { // --z--> k
						int k = sk.second;
						binaryRecord[i][k].insert(make_fast_pair(ind, j));
						if (!hasEdge(i, x, k)) {
							addEdge(i, x, k);
							w.push_back(make_edge(i, x, k)); 
						}
					}
				}
			}
			for (int ind : g.binaryProductionsSecondInv[y]) { // x -> zy
				auto &p = g.binaryProductions[ind];
				int x = p.first, z = p.second.first;
				for (auto &ks : counterAdjacencyVector[i]) {
					if (ks.second == z) { // k --z-->
						int k = ks.first;
						binaryRecord[k][j].insert(make_fast_pair(ind, i));
						if (!hasEdge(k, x, j)) {
							addEdge(k, x, j);
							w.push_back(make_edge(k, x, j));
						}
					}
				}
			}
		}
	}

	/*
	unordered_set<int> getCFLReachabilityVertexClosure(int i, int j, const Grammar &g) const { // unoptimized
		if (!hasEdge(i, g.startSymbol, j)) {
			return unordered_set<int>();
		} else {
			unordered_set<int> ret {i, j};
			Edge start = make_edge(i, g.startSymbol, j);
			set<Edge> vis {start};
			deque<Edge> q {start};
			while (!q.empty()) { // BFS
				Edge cur = q.front();
				q.pop_front();

				// i --x--> j
				int i = cur.first.first, j = cur.first.second, x = cur.second;

				if (unaryRecord[i].count(j) == 1) {
					for (int ind : unaryRecord[i].at(j)) {
						if (g.unaryProductions[ind].first == x) {
							Edge nxt = make_edge(i, g.unaryProductions[ind].second, j);
							if (vis.count(nxt) == 0) {
								vis.insert(nxt);
								q.push_back(nxt);
							}
						}
					}
				}
				if (binaryRecord[i].count(j) == 1) {
					for (long long fp : binaryRecord[i].at(j)) {
						auto p = unpack_fast_pair(fp);
						int ind = p.first, k = p.second; // i --> k --> j
						if (g.binaryProductions[ind].first == x) {
							ret.insert(k);
							Edge nxt1 = make_edge(i, g.binaryProductions[ind].second.first, k);
							Edge nxt2 = make_edge(k, g.binaryProductions[ind].second.second, j);
							if (vis.count(nxt1) == 0) {
								vis.insert(nxt1);
								q.push_back(nxt1);
							}
							if (vis.count(nxt2) == 0) {
								vis.insert(nxt2);
								q.push_back(nxt2);
							}
						}
					}
				}
			}
			return ret;
		}
	}
	*/

	// TODO: precision?
	set<Edge> getCFLReachabilityEdgeClosure(int i, int j, const Grammar &g) const {
		set<Edge> closure;
		set<Edge> vis;
		deque<Edge> q;
		if (hasEdge(i, g.startSymbol, j)) {
			Edge start = make_edge(i, g.startSymbol, j);
			vis.insert(start);
			q.push_back(start);
		}
		while (!q.empty()) { // BFS
			Edge cur = q.front();
			q.pop_front();

			// i --x--> j
			int i = cur.first.first, j = cur.first.second, x = cur.second;
			if (g.nonterminals.count(x) == 0) { // terminals or negligible symbols
				closure.insert(make_edge(i, x, j));
			}

			if (negligibleRecord[i].count(j) == 1) {
				if (x == g.startSymbol) {
					for (int s : negligibleRecord[i].at(j)) {
						Edge nxt = make_edge(i, s, j);
						if (vis.count(nxt) == 0) {
							vis.insert(nxt);
							q.push_back(nxt);
						}
					}
				}
			}
			if (unaryRecord[i].count(j) == 1) {
				for (int ind : unaryRecord[i].at(j)) {
					if (g.unaryProductions[ind].first == x) {
						Edge nxt = make_edge(i, g.unaryProductions[ind].second, j);
						if (vis.count(nxt) == 0) {
							vis.insert(nxt);
							q.push_back(nxt);
						}
					}
				}
			}
			if (binaryRecord[i].count(j) == 1) {
				for (long long fp : binaryRecord[i].at(j)) {
					auto p = unpack_fast_pair(fp);
					int ind = p.first, k = p.second; // i --> k --> j
					if (g.binaryProductions[ind].first == x) {
						Edge nxt1 = make_edge(i, g.binaryProductions[ind].second.first, k);
						Edge nxt2 = make_edge(k, g.binaryProductions[ind].second.second, j);
						if (vis.count(nxt1) == 0) {
							vis.insert(nxt1);
							q.push_back(nxt1);
						}
						if (vis.count(nxt2) == 0) {
							vis.insert(nxt2);
							q.push_back(nxt2);
						}
					}
				}
			}
		}
		return closure;
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
	gm.fillInv(11);
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
			if ((i == j) || (i == 0 && j == 2) || (i == 0 && j == 3) ||
			    (i == 2 && j == 3) || (i == 4 && j == 5)) {
				assert(gh.hasEdge(i, gm.startSymbol, j));
			} else {
				assert(!gh.hasEdge(i, gm.startSymbol, j));
			}
		}
	}
	/*
	auto rvc1 = gh.getCFLReachabilityVertexClosure(0, 2, gm);
	assert(rvc1.size() == 5);
	assert(rvc1.count(0) == 1);
	assert(rvc1.count(1) == 1);
	assert(rvc1.count(2) == 1);
	assert(rvc1.count(4) == 1);
	assert(rvc1.count(5) == 1);
	auto rvc2 = gh.getCFLReachabilityVertexClosure(2, 3, gm);
	assert(rvc2.size() == 2);
	assert(rvc2.count(2) == 1);
	assert(rvc2.count(3) == 1);
	auto rvc3 = gh.getCFLReachabilityVertexClosure(2, 2, gm);
	assert(rvc3.size() == 1);
	assert(rvc3.count(2) == 1);
	*/
	auto rec1 = gh.getCFLReachabilityEdgeClosure(0, 2, gm);
	assert(rec1.size() == 5);
	assert(rec1.count(make_edge(0, 1, 1)) == 1);
	assert(rec1.count(make_edge(1, 2, 2)) == 1);
	assert(rec1.count(make_edge(0, 1, 4)) == 1);
	assert(rec1.count(make_edge(4, 10, 5)) == 1);
	assert(rec1.count(make_edge(5, 2, 2)) == 1);
	auto rec2 = gh.getCFLReachabilityEdgeClosure(2, 3, gm);
	assert(rec2.size() == 2);
	assert(rec2.count(make_edge(2, 1, 2)) == 1);
	assert(rec2.count(make_edge(2, 2, 3)) == 1);
	auto rec3 = gh.getCFLReachabilityEdgeClosure(2, 2, gm);
	assert(rec3.size() == 0);
}

bool isEdgeLine(const string &line) {
	for (char c : line) {
		if (c == '>') {
			return true;
		}
	}
	return false;
}

pair<pair<int, int>, pair<string, int>> parseLine(string line) {
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
	return make_pair(make_pair(v1, v2), make_pair(label.substr(0, 2), stoi(label.substr(4))));
}

map<int, int> normalizeNumbers(int start, const vector<int> &numbers) {
	map<int, int> m;
	for (int n : numbers) {
		m[n] = 0;
	}
	int i = start;
	for (auto &pr : m) {
		pr.second = i++;
	}
	return m;
}

pair<pair<vector<Edge>, pair<int, int>>, vector<Grammar>> readFile(string fname) {
	ifstream in(fname); // file auto closed via destructor

	// read raw edges
	string line;
	vector<pair<pair<int, int>, pair<string, int>>> rawEdges;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			rawEdges.push_back(parseLine(line));
		}
	}

	// vertices
	// 0, 1, ..., n - 1

	// normalize vertices
	vector<int> v;
	for (auto &ijtn : rawEdges) {
		v.push_back(ijtn.first.first);
		v.push_back(ijtn.first.second);
	}
	auto nv_map = normalizeNumbers(0, v);
	int n = nv_map.size();

	// symbols
	// [ first Dyck's terminals     ]    [ second Dyck's terminals    ]    [ first Dyck's nonterminals    ]    [ second Dyck's nonterminals   ]
	// (_1, ..., (_n1, )_1, ..., )_n1    [_1, ..., [_n2, ]_1, ..., ]_n2    D, D_1, D_2, D_3, D_4, ..., D_n1    E, E_1, E_2, E_3, E_4, ..., E_n2

	// normalize labels
	vector<int> p;
	vector<int> b;
	for (auto &ijtn : rawEdges) {
		if (ijtn.second.first == "op" || ijtn.second.first == "cp") {
			p.push_back(ijtn.second.second);
		} else {
			b.push_back(ijtn.second.second);
		}
	}
	auto np_map = normalizeNumbers(0, p);
	int n1 = np_map.size();
	auto nb_map = normalizeNumbers(2 * n1, b);
	int n2 = nb_map.size();

	int start = 2 * n1 + 2 * n2 - 1;
	Grammar gmp, gmb;
	auto fillDyck = [](Grammar &gm, int op_begin, int n, int start) -> void {
		for (int i = op_begin; i < op_begin + 2 * n; i++) {
			gm.terminals.insert(i);
		}
		for (int i = start; i <= start + n; i++) {
			gm.nonterminals.insert(i);
		}
		gm.emptyProductions.push_back(start);
		gm.binaryProductions.push_back(make_pair(start, make_pair(start, start)));
		for (int i = 0; i < n; i++) {
			gm.binaryProductions.push_back(make_pair(start, make_pair(op_begin + i, start + 1 + i)));
			gm.binaryProductions.push_back(make_pair(start + 1 + i, make_pair(start, op_begin + n + i)));
		}
		gm.startSymbol = start;
	};
	fillDyck(gmp, 0, n1, 2 * n1 + 2 * n2);
	fillDyck(gmb, 2 * n1, n2, 2 * n1 + 2 * n2 + n1 + 1);
	int total = 2 * n1 + 2 * n2 + n1 + 1 + n2 + 1;
	gmp.fillInv(total);
	gmb.fillInv(total);

	if (n > FP_MASK) {
		cerr << "Error: The graph contains too many nodes." << endl;
		exit(EXIT_FAILURE);
	}

	if (total > FP_MASK) {
		cerr << "Error: The grammar contains too many symbols." << endl;
		exit(EXIT_FAILURE);
	}

	vector<Edge> edges;
	for (auto &ijtn : rawEdges) {
		string t = ijtn.second.first;
		int n = ijtn.second.second;
		int sym;
		if (t == "op") {
			sym = np_map[n];
		} else if (t == "cp") {
			sym = np_map[n] + n1;
		} else if (t == "ob") {
			sym = nb_map[n];
		} else {
			sym = nb_map[n] + n2;
		}
		edges.push_back(make_edge(nv_map[ijtn.first.first], sym, nv_map[ijtn.first.second]));
	}

	return make_pair(make_pair(edges, make_pair(0, n - 1)), vector<Grammar> {gmp, gmb});
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		test();
	} else {
		// retrieve data
		const pair<pair<vector<Edge>, pair<int, int>>, vector<Grammar>> data = readFile(argv[1]);
		const vector<Edge> &edges = data.first.first;
		const vector<Grammar> &grammars = data.second;
		const int n = data.first.second.second + 1;

		// construct graphs
		Graph gh1(n);
		gh1.fillEdges(edges);
		Graph gh2 = gh1;

		// run CFL reachability
		// cout << ">>> Running CFL Reachability" << endl;
		// auto start = std::chrono::steady_clock::now();
		gh1.runCFLReachability(grammars[0]);
		gh2.runCFLReachability(grammars[1]);
		// auto end = std::chrono::steady_clock::now();
		// std::chrono::duration<double> elapsed_seconds = end - start;
		// cout << ">>> CFL Reachability Done. Time (Seconds): " << elapsed_seconds.count() << endl;

		// if (argc == 3) {
		// 	return 0;
		// }

		// main query loop
		int totalCFL1 = 0;
		int totalCFL2 = 0;
		int totalCFLBoolean = 0;
		int totalEC = 0;
		int totalECFix = 0;
		for (int source = 0; source < n; source++) {
			cout << ">>> [main] Query Progress (Source Vertex): " << source << ',' << n - 1 << endl;
			for (int sink = 0; sink < n; sink++) {
				bool reach1 = gh1.hasEdge(source, grammars[0].startSymbol, sink);
				bool reach2 = gh2.hasEdge(source, grammars[1].startSymbol, sink);
				if (reach1) {
					totalCFL1++;
				}
				if (reach2) {
					totalCFL2++;
				}
				if (reach1 && reach2) {
					totalCFLBoolean++;
					
					// EC
					auto c1 = gh1.getCFLReachabilityEdgeClosure(source, sink, grammars[0]);
					auto c2 = gh2.getCFLReachabilityEdgeClosure(source, sink, grammars[1]);
					set<Edge> c;
					for (auto &e : c1) {
						if (c2.count(e) == 1) {
							c.insert(e);
						}
					}
					Graph gh(n);
					gh.fillEdges(c);
					if (gh.runPureReachability(source, sink)) {
						totalEC++;
					}

					// ECFix
					set<Edge> es = c;
					while (true) {
						Graph gh(n);
						gh.fillEdges(es);
						gh.runCFLReachability(grammars[0]);
						if (!(gh.hasEdge(source, grammars[0].startSymbol, sink))) {
							break;
						}
						auto c1 = gh.getCFLReachabilityEdgeClosure(source, sink, grammars[0]);
						gh = Graph(n);
						gh.fillEdges(c1);
						gh.runCFLReachability(grammars[1]);
						if (!(gh.hasEdge(source, grammars[1].startSymbol, sink))) {
							break;
						}
						auto c2 = gh.getCFLReachabilityEdgeClosure(source, sink, grammars[1]);
						if (c2.size() == es.size()) {
							totalECFix++;
							break;
						} else {
							es = c2;
						}
					}
				}
			}
		}
		
		cout << "totalCFL1: " << totalCFL1 << endl;
		cout << "totalCFL2: " << totalCFL2 << endl;
		cout << "totalCFLBoolean: " << totalCFLBoolean << endl;
		cout << "totalEC: " << totalEC << endl;
		cout << "totalECFix: " << totalECFix << endl;
	}
	return 0;
}
