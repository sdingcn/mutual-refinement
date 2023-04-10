//===- svf-ex.cpp -- A driver example of SVF-------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013->  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===-----------------------------------------------------------------------===//

/*
 // A driver program of SVF including usages of SVF APIs
 //
 // Author: Yulei Sui,
 */

#include "SVF-FE/LLVMUtil.h"
#include "Graphs/SVFG.h"
#include "WPA/Andersen.h"
#include "SVF-FE/PAGBuilder.h"
#include "Graphs/ConsG.h"
#include "Util/SCC.h"
#include "MSSA/MemSSA.h"

namespace SVF
{
class ReachabilityGraph: public ConstraintGraph {
	std::map<const ConstraintEdge*, NodeID> callToId;
	std::map<const ConstraintEdge*, NodeID> retToId;
	std::set<ConstraintEdge*> otherEdges;
public:
	ReachabilityGraph(PAG *pag) : ConstraintGraph(pag) {
		// label each call with a unique id from the callsite
		PAGEdge::PAGEdgeSetTy& calls = getPAGEdgeSet(PAGEdge::Call);
		for (PAGEdge::PAGEdgeSetTy::iterator it = calls.begin(), eit = calls.end(); it != eit; ++it) {
			CallPE *pagEdge = SVFUtil::dyn_cast<CallPE>(*it);
			ConstraintEdge *cgEdge = getEdge(
					getConstraintNode(pagEdge->getSrcID()),
					getConstraintNode(pagEdge->getDstID()),
					ConstraintEdge::Copy);
			callToId[cgEdge] = pagEdge->getCallSite()->getId();
		}
		// label each return with a unique id from the callsite
		PAGEdge::PAGEdgeSetTy& rets = getPAGEdgeSet(PAGEdge::Ret);
		for (PAGEdge::PAGEdgeSetTy::iterator it = rets.begin(), eit = rets.end(); it != eit; ++it) {
			RetPE *pagEdge = SVFUtil::dyn_cast<RetPE>(*it);
			ConstraintEdge *cgEdge = getEdge(
					getConstraintNode(pagEdge->getSrcID()),
					getConstraintNode(pagEdge->getDstID()),
					ConstraintEdge::Copy);
			retToId[cgEdge] = pagEdge->getCallSite()->getId();
		}
		// collect and remove all address edges
		std::set<AddrCGEdge*> addrEdges;
		for (ConstraintEdge::ConstraintEdgeSetTy::iterator it = AddrCGEdgeSet.begin(), eit = AddrCGEdgeSet.end(); it != eit; ++it) {
			AddrCGEdge *ae = SVFUtil::dyn_cast<AddrCGEdge>(*it);
			addrEdges.insert(ae);
		}
		for (std::set<AddrCGEdge*>::iterator it = addrEdges.begin(), eit = addrEdges.end(); it != eit; ++it) {
			removeAddrEdge(*it);
		}
	}

	inline bool isCallEdge(const ConstraintEdge* edge) const {
		return callToId.find(edge) != callToId.end();
	}

	inline bool isRetEdge(const ConstraintEdge* edge) const {
		return retToId.find(edge) != retToId.end();
	}

	// dump graph into dot file
	void dump(std::string name) {
		llvm::outs() << "Writing graph to " << name << "...\n";
		// error handling
		std::error_code err;
		llvm::ToolOutputFile outFile(name, err, llvm::sys::fs::F_None);
		if (err) {
			llvm::outs() << "Error occured!\n";
			outFile.os().clear_error();
			return;
		}
		// for relabelling
		std::map<NodeID, long long> pMap;
		std::set<NodeID> ids;
		for (auto &p : callToId) {
			ids.insert(p.second);
		}
		for (auto &p : retToId) {
			ids.insert(p.second);
		}
		for (auto &i : ids) {
			pMap[i] = pMap.size();
		}
		for (ConstraintGraph::iterator it = begin(), eit = end(); it != eit; ++it) {
			// "it" iterates over an id-to-node map
			ConstraintNode *node = it->second;
			for (ConstraintNode::iterator cit = node->OutEdgeBegin(), ecit = node->OutEdgeEnd(); cit != ecit; ++cit) {
				ConstraintEdge *edge = *cit;
				if (LoadCGEdge *load = SVFUtil::dyn_cast<LoadCGEdge>(edge)) {
					outFile.os() << edge->getSrcID() << "->" << edge->getDstID() << "[label=\"cb--0\"]\n";
				} else if (StoreCGEdge *store = SVFUtil::dyn_cast<StoreCGEdge>(edge)) {
					outFile.os() << edge->getSrcID() << "->" << edge->getDstID() << "[label=\"ob--0\"]\n";
				} else {
					if (isCallEdge(edge)) {
						long long label = pMap[callToId.at(edge)];
						outFile.os() << edge->getSrcID() << "->" << edge->getDstID() << "[label=\"op--" << label << "\"]\n";
					} else if (isRetEdge(edge)) {
						long long label = pMap[retToId.at(edge)];
						outFile.os() << edge->getSrcID() << "->" << edge->getDstID() << "[label=\"cp--" << label << "\"]\n";
					} else {
						outFile.os() << edge->getSrcID() << "->" << edge->getDstID() << "[label=\"normal\"]\n";
					}
				}
			}
		}
		// close file
		outFile.os().close();
		if (!outFile.os().has_error()) {
			llvm::outs() << "\n";
			outFile.keep(); // if you don't call keep() the file will be deleted when the ToolOutputFile object is destroyed
		}
	}
};
} // end namespace SVF

static llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
        llvm::cl::desc("<input bitcode>"), llvm::cl::init("-"));

int main(int argc, char **argv) {
    int arg_num = 0;
    char **arg_value = new char*[argc];
    std::vector<std::string> moduleNameVec;
    SVF::SVFUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);
    llvm::cl::ParseCommandLineOptions(arg_num, arg_value, "Whole Program Points-to Analysis\n");
    SVF::SVFModule *svfModule = SVF::LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);

    // build Program Assignment Graph (PAG)
    SVF::PAGBuilder builder;
    SVF::PAG *pag = builder.build(svfModule);

    // generate Reachability Graph
    SVF::ReachabilityGraph rg(pag);
    rg.dump("graph.dot");

    // don't need to clean up memory since the program ends.
    return 0;
}
