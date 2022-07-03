#include "lcllib.h"

#include <assert.h>
#include <sys/time.h>
#include <bitset>
#include <cstring>
#include <fstream>
#include <iostream>
#include <queue>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <tuple>
#include <utility>
#include "CFLReach.h"
#include "bitmap.h"

void runLCL(std::stringstream &graph, std::set<std::pair<std::string, std::string>> &reachablePairs,
		bool trace, std::set<std::tuple<std::string, std::string, std::string>> &contributingEdges) {
	// parse the raw edge set
	SimpleDotParser dotparser;
	std::unordered_map<std::string, unsigned> NodeID;
	std::unordered_map<unsigned, std::string> NodeID_R;
	unsigned NodeNum = dotparser.BuildNodeMap(graph, NodeID, NodeID_R);
	// do the LCL computation
	std::vector<std::vector<std::unordered_map<LinEdgeTy, std::unordered_set<LinEdgeTy>>>> trace_L(NodeNum);
	std::vector<std::vector<std::unordered_map<LinEdgeTy, std::unordered_set<LinEdgeTy>>>> trace_R(NodeNum);
	CFLMatrixLin cm1(NodeNum);
	bitmap_obstack_initialize(NULL);
	bitmap* S[NodeNum];
	std::vector<std::vector<std::unordered_set<std::string>>> orig_graph(NodeNum);
	long **observed  = (long **)std::malloc(sizeof(long *)*NodeNum);
	long **goodq2 = (long **)std::malloc(sizeof(long *)*NodeNum);
	LCLReach(NodeID, NodeID_R, NodeNum, graph, trace_L, trace_R, cm1, S, orig_graph, observed, goodq2, trace);
	unsigned q2_mLin = q2Lin*numLinEdgeTy + mLin;
	// write reachable pairs
	for (int i = 0; i < NodeNum; i++) {
		reachablePairs.insert(std::make_pair(NodeID_R[i], NodeID_R[i]));
	}
	for (unsigned ii = 0; ii < NodeNum; ii++) {
		for (unsigned jj = 0; jj < NodeNum; jj++) {
			unsigned NodeS = ii;
			unsigned NodeT = jj;
			if (NodeS != NodeT) {
				if (TestItemInSet(observed[NodeS], NodeT) &&
				    bitmap_bit_p(S[NodeS][NodeT], q2_mLin) &&
				    TestItemInSet(goodq2[NodeS], NodeT)) {
					reachablePairs.insert(std::make_pair(NodeID_R[NodeS], NodeID_R[NodeT]));
				}
			}
		}
	}
	// (if required) write all pairs contributing edges
	if (trace) {
		std::vector<std::vector<std::unordered_set<std::string>>> output(NodeNum, std::vector<std::unordered_set<std::string>>(NodeNum));
		std::vector<std::vector<std::unordered_set<LinEdgeTy>>> visited(NodeNum, std::vector<std::unordered_set<LinEdgeTy>>(NodeNum));
		for (unsigned ii = 0; ii < NodeNum; ii++) {
			for (unsigned jj = 0; jj < NodeNum; jj++) {
				unsigned NodeS = ii;
				unsigned NodeT = jj;
				if (TestItemInSet(observed[NodeS], NodeT) &&
				    bitmap_bit_p(S[NodeS][NodeT], q2_mLin) &&
				    TestItemInSet(goodq2[NodeS], NodeT)) {
					dfs(NodeS, NodeT, q2_mLin, trace_L, trace_R, cm1, visited, S, observed, orig_graph, NodeID_R, output);
				}
			}
		}
		for (unsigned i = 0; i < NodeNum; i++) {
			for (unsigned j = 0; j < NodeNum; j++) {
				if (output[i][j].size() > 0) {
					for (auto x : output[i][j]) {
						contributingEdges.insert(std::make_tuple(NodeID_R[i], x, NodeID_R[j]));
					}
				}
			}
		}
	}
	free(observed);
	free(goodq2);
}
