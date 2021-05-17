#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <deque>
#include <cassert>
#include <string>
#include <algorithm>
#include <chrono>

using std::pair;
using std::make_pair;
using std::vector;
using std::set;
using std::map;
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

struct Grammar {
	set<int> terminals;
	set<int> nonterminals;
	vector<int> emptyProductions;
	vector<pair<int, int>> unaryProductions;
	vector<pair<int, pair<int, int>>> binaryProductions;
	int startSymbol;
	/*
	void print() {
		for (int e : emptyProductions) {
			cerr << e << " -> " << 'e' << endl;
		}
		for (auto &ab : unaryProductions) {
			cerr << ab.first << " -> " << ab.second << endl;
		}
		for (auto &abc : binaryProductions) {
			cerr << abc.first << " -> " << abc.second.first << ' ' << abc.second.second << endl;
		}
	}
	*/
};

using Edge = pair<pair<int, int>, int>; // ((first vertex, second vertex), label)

Edge make_edge(int i, int x, int j) {
	return make_pair(make_pair(i, j), x);
}

struct Graph {
	int numberOfVertices;

	// edges
	vector<set<pair<int, int>>> fastEdgeTest; // first vertex -> {(label, second vertex)}
	vector<vector<pair<int, int>>> adjacencyVector; // first vertex -> [(label, second vertex)]
	vector<vector<pair<int, int>>> counterAdjacencyVector; // second vertex -> [(first vertex, label)]

	// records for reachability closures
	vector<map<int, set<int>>> negligibleRecord; // i -> j -> {negligible symbol}
	vector<map<int, set<int>>> unaryRecord; // i -> j -> {unary production number}
	vector<map<int, set<pair<int, int>>>> binaryRecord; // i -> j -> {(binary prodution number, middle vertex)}

	Graph(int n) : numberOfVertices(n),
                       fastEdgeTest(n), adjacencyVector(n), counterAdjacencyVector(n),
                       negligibleRecord(n), unaryRecord(n), binaryRecord(n) {}

	void addEdge(int i, int x, int j) { // i --x--> j
		fastEdgeTest[i].insert(make_pair(x, j));
		adjacencyVector[i].push_back(make_pair(x, j));
		counterAdjacencyVector[j].push_back(make_pair(i, x));
	}

	bool hasEdge(int i, int x, int j) const {
		return fastEdgeTest[i].count(make_pair(x, j)) == 1;
	}

	bool runPureReachability(int i, int j) {
		deque<int> q;
		set<int> s;
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
		deque<Edge> w; // ((first vertex, second vertex), label)
		vector<Edge> negligibleEdges;
		for (int i = 0; i < numberOfVertices; i++) { // add all edges to the worklist, and find out all negligible edges
			for (auto &sj : adjacencyVector[i]) { // --s--> j
				w.push_back(make_edge(i, sj.first, sj.second));
				if (g.terminals.count(sj.first) == 0) {
					negligibleEdges.push_back(make_edge(i, sj.first, sj.second));
				}
			}
		}
		for (auto &e : negligibleEdges) { // add negligible edges to the edge set and the worklist
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

			for (int ind = 0; ind < nup; ind++) {
				auto &p = g.unaryProductions[ind];
				if (p.second == y) { // x -> y
					int x = p.first;
					unaryRecord[i][j].insert(ind);
					if (!hasEdge(i, x, j)) {
						addEdge(i, x, j);
						w.push_back(make_edge(i, x, j));
					}
				}
			}
			for (int ind = 0; ind < nbp; ind++) {
				auto &p = g.binaryProductions[ind];
				if (p.second.first == y) { // x -> yz
					int x = p.first, z = p.second.second;
					for (auto &sk : adjacencyVector[j]) { // --s--> k
						if (sk.first == z) { // --z--> k
							int k = sk.second;
							binaryRecord[i][k].insert(make_pair(ind, j));
							if (!hasEdge(i, x, k)) {
								addEdge(i, x, k);
								w.push_back(make_edge(i, x, k)); 
							}
						}
					}
				}
				if (p.second.second == y) { // x -> zy
					int x = p.first, z = p.second.first;
					for (auto &ks : counterAdjacencyVector[i]) {
						if (ks.second == z) { // k --z-->
							int k = ks.first;
							binaryRecord[k][j].insert(make_pair(ind, i));
							if (!hasEdge(k, x, j)) {
								addEdge(k, x, j);
								w.push_back(make_edge(k, x, j));
							}
						}
					}
				}
			}
		}
	}

	set<int> getCFLReachabilityVertexClosure(int i, int j, const Grammar &g) const {
		if (!hasEdge(i, g.startSymbol, j)) {
			return set<int>();
		} else {
			set<int> ret {i, j};
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
					for (auto &ind_k : binaryRecord[i].at(j)) {
						int ind = ind_k.first, k = ind_k.second; // i --> k --> j
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

	set<Edge> getCFLReachabilityEdgeClosure(int i, int j, const Grammar &g) const {
		if (!hasEdge(i, g.startSymbol, j)) {
			return set<Edge>();
		} else {
			set<Edge> ret;
			Edge start = make_edge(i, g.startSymbol, j);
			set<Edge> vis {start};
			deque<Edge> q {start};
			while (!q.empty()) { // BFS
				Edge cur = q.front();
				q.pop_front();

				// i --x--> j
				int i = cur.first.first, j = cur.first.second, x = cur.second;
				if (g.terminals.count(x) == 1 ||
				   (g.terminals.count(x) == 0 && g.nonterminals.count(x) == 0)) {
					ret.insert(make_edge(i, x, j));
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
					for (auto &ind_k : binaryRecord[i].at(j)) {
						int ind = ind_k.first, k = ind_k.second; // i --> k --> j
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

// (([edge], (source, sink)), [grammar])
// Vertices should be normalized (0, 1, ..., n - 1 (n >= 1)).
// Symbols in grammars should be integers.
// pay attention to the grammars' order
pair<pair<vector<Edge>, pair<int, int>>, vector<Grammar>> readFile(string fname) {
	ifstream in(fname); // This file will be automatically closed during the call of the destructor of "in".

	// read raw edges
	string line;
	vector<pair<pair<int, int>, pair<string, int>>> rawEdges;
	while (getline(in, line)) {
		if (isEdgeLine(line)) {
			rawEdges.push_back(parseLine(line));
		}
	}

	// normalize vertices
	map<int, int> m;
	for (auto &ijtn : rawEdges) {
		m[ijtn.first.first] = 0;
		m[ijtn.first.second] = 0;
	}
	int ind = 0; // number of vertices
	for (auto &pr : m) {
		pr.second = ind++;
	}
	vector<pair<int, int>> nes;
	for (auto &ijtn : rawEdges) {
		nes.push_back(make_pair(m[ijtn.first.first], m[ijtn.first.second]));
	}
	// vertices: [0, ind - 1]

	// normalize labels
	map<int, int> mp;
	map<int, int> mb;
	for (auto &ijtn : rawEdges) {
		if (ijtn.second.first == "op" ||
                    ijtn.second.first == "cp") {
			mp[ijtn.second.second] = 0;
		} else {
			mb[ijtn.second.second] = 0;
		}
	}
	int indp = 0; // number of types of parentheses
	for (auto &pr : mp) {
		pr.second = ++indp; // avoid 0 here, since later we want both x and -x
	}
	int indb = 0; // number of types of brackets
	for (auto &pr : mb) {
		pr.second = ++indb;
	}
	vector<int> nls;
	for (auto &ijtn : rawEdges) {
		if (ijtn.second.first == "op") {
			nls.push_back(mp[ijtn.second.second]);
		} else if (ijtn.second.first == "cp") {
			nls.push_back(-mp[ijtn.second.second]);
		} else if (ijtn.second.first == "ob") {
			nls.push_back(indp + mb[ijtn.second.second]);
		} else {
			nls.push_back(-(indp + mb[ijtn.second.second]));
		}
	}
	// [-indp - indb, -indp - 1] [-indp, -1] [1, indp] [indp + 1, indp + indb]

	int avlb = indp + indb + 10; // the next available number
	Grammar gmp, gmb;
	auto fillDyck = [](Grammar &gm, int bg /* open parenthesis number begin */,
			                int ed /* open parenthesis number end */, int &avlb) -> void {
		// (-ed, -bg] [bg, ed) ... [avlb, avlb + 1) [avlb + 1, avlb + 1 + ed - bg)
		for (int i = bg; i < ed; i++) {
			gm.terminals.insert(i);
			gm.terminals.insert(-i);
		}
		for (int i = avlb; i < avlb + 1 + ed - bg; i++) {
			gm.nonterminals.insert(i);
		}
		gm.emptyProductions.push_back(avlb);
		gm.binaryProductions.push_back(make_pair(avlb, make_pair(avlb, avlb)));
		for (int i = bg; i < ed; i++) {
			gm.binaryProductions.push_back(make_pair(avlb, make_pair(i, i - bg + avlb + 1)));
			gm.binaryProductions.push_back(make_pair(i - bg + avlb + 1, make_pair(avlb, -i)));
		}
		gm.startSymbol = avlb;
		avlb = avlb + 1 + ed - bg;
	};
	fillDyck(gmp, 1, indp + 1, avlb);
	fillDyck(gmb, indp + 1, indp + indb + 1, avlb);

	vector<Edge> retEdges;
	int ne = rawEdges.size();
	for (int i = 0; i < ne; i++) {
		retEdges.push_back(make_pair(nes[i], nls[i]));
	}

	return make_pair(make_pair(retEdges, make_pair(0, ind - 1)), vector<Grammar> {gmp, gmb});
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		test();
	} else {
		// retrieve data
		pair<pair<vector<Edge>, pair<int, int>>, vector<Grammar>> data = readFile(argv[1]);
		vector<Edge> &edges = data.first.first;
		vector<Grammar> &grammars = data.second;
		int n = data.first.second.second + 1;

		// construct graphs
		Graph gh1(n);
		for (auto &ijs : edges) {
			gh1.addEdge(ijs.first.first, ijs.second, ijs.first.second);
		}
		Graph gh2 = gh1;

		// run CFL reachability
		cout << ">>> Running CFL Reachability" << endl;
		auto start = std::chrono::steady_clock::now();
		gh1.runCFLReachability(grammars[0]);
		gh2.runCFLReachability(grammars[1]);
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		cout << ">>> CFL Reachability Done. Time (Seconds): " << elapsed_seconds.count() << endl;
		return 0;

		// main query loop
		int total = 0;
		int total1 = 0;
		int total2 = 0;
		for (int source = 0; source < n; source++) {
			cout << ">>> Query Progress (Source Vertex): " << source << ',' << n - 1 << endl;
			for (int sink = 0; sink < n; sink++) {
				if (gh1.hasEdge(source, grammars[0].startSymbol, sink)) {
					total1++;
				}
				if (gh2.hasEdge(source, grammars[1].startSymbol, sink)) {
					total2++;
				}
				auto c1 = gh1.getCFLReachabilityEdgeClosure(source, sink, grammars[0]);
				auto c2 = gh2.getCFLReachabilityEdgeClosure(source, sink, grammars[1]);
				set<pair<pair<int, int>, int>> c;
				for (auto e : c1) {
					if (c2.count(e) == 1) {
						c.insert(e);
					}
				}
				Graph gh(n);
				for (auto &ijs : c) {
					gh.addEdge(ijs.first.first, ijs.second, ijs.first.second);
				}
				if (gh.runPureReachability(source, sink)) {
					total++;
				}
			}
		}
		cout << "total: " << total << endl;
		cout << "total1: " << total1 << endl;
		cout << "total2: " << total2 << endl;
		//int rounds = grammars.size(); // the number of CFLs that we want to intersect
		///**/for (int se = 0; se < n; se++) {
		///**/for (int sk = 0; sk < n; sk++) {
		//cerr << "Source: " << se << " Sink: " << sk << endl;
		//set<int> vertices; // the vertices that we want to consider in each round (TODO: consider changing this to a Boolean array?)
		//for (int i = 0; i < n; i++) { // In the first round, we consider all vertices.
		//	vertices.insert(i);
		//}
		//for (int r = 0; r < rounds; r++) {
		//	// cerr << "Starting Round " << r << endl;
		//	Graph gh(n);
		//	for (auto &ijs : edges) { // We only include edges containing vertices that we want to consider.
		//		if (vertices.count(ijs.first.first) == 1 &&
		//		    vertices.count(ijs.first.second) == 1) {
		//			gh.addEdge(ijs.first.first, ijs.second, ijs.first.second);
		//		}
		//	}
		//	gh.runCFLReachability(grammars[r]);
		//	vertices = gh.getCFLReachabilityClosure(se, sk, grammars[r]);
		//	if (vertices.size() == 0) {
		//		// cout << "Definitely Unreachable" << endl;
		//		goto NX;
		//	}
		//}
		//cout << "**********> Possibly Reachable " << se << "->" << sk << endl;
//NX:
		//;
		///**/}
		///**/}
	}
	return 0;
}
