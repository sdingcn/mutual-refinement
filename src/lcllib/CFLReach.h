#ifndef CFLREACH_H
#define CFLREACH_H


#include "CFLGraph.h"
#include <iostream>
#include <fstream>
#include <utility>
#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <bitset>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include "bitmap.h"

using namespace std;

//enum FromOrTo {f, t}; 
//enum edgeTy {D, A};
//enum EdgeTy {non, EdgeTyD, EdgeTyA, M, DV, V, MAM, M_q, MA_s, MA, AM_s, AM, Ab, Db, e, numEdgeTy}; //note non means no edge, e means empty edge.



//unordered_map<string, EdgeTy> EdgeMap;
//unsigned S_edge = 0;

//ifstream in("dotfile/popl");
//long *isst;
//long *ised;

//typedef unsigned int       u32;
inline int TestItemInSet(long *p, unsigned item){
  //int section  = item / 64;
  //int offset = item % 64;
  //unsigned long base  = 1;
  p = p + item / 64;
  
  if (((*p) & (1UL << (item % 64))) == 0)
    return 0;
  else
    return 1;

  



}

inline void AddItemToSet(long *p, unsigned item){
  //int section = item / 64;
  //int offset = item % 64;
  //unsigned long base = 1;
  

  p = p + item / 64;
  *p = (*p) | (1UL << (item % 64));
  
  //return 0;

}







  
#define MEM_USAGE() {                                  \
    string mline; ifstream min("/proc/self/status");     \
    for (unsigned i = 0; i < 16; i++) { getline(min,mline); } \
    istringstream inl(mline); string x; unsigned mem;        \
    inl >> x >> mem; cout << "mem  = " <<		     \
    (double)mem/1024 << "M" << endl; min.close();       \
}





class ReachabilityItem{
 public:
 ReachabilityItem(unsigned a, unsigned b, unsigned c) : vtx1(a), vtx2(b), edgeTy(c){};
  
  unsigned GetEdgeTy() const {return edgeTy;}
  unsigned GetVtx1() const {return vtx1;}
  unsigned GetVtx2() const {return vtx2;}
  void SetEdgeTy(unsigned edg1) {edgeTy = edg1;}
  void SetVtx1(unsigned a) {vtx1 = a;}
  void SetVtx2(unsigned b) {vtx2 = b;}
 private:
  unsigned vtx1;
  unsigned vtx2;
  unsigned edgeTy;
};

class ReachabilityItemLin{
 public:
 ReachabilityItemLin(unsigned a, unsigned b, unsigned c, char d, unsigned e, unsigned f) : vtx1(a), vtx2(b), edgeTy(c), good(d), fb_l(e), fb_r(f){};
  

  unsigned GetFBR() const {return fb_r;}
  unsigned GetFBL() const {return fb_l;}
  unsigned GetEdgeTy() const {return edgeTy;}
  unsigned GetVtx1() const {return vtx1;}
  unsigned GetVtx2() const {return vtx2;}
  char IsGood() const {return good;}
  void SetEdgeTy(unsigned edg1) {edgeTy = edg1;}
  void SetVtx1(unsigned a) {vtx1 = a;}
  void SetVtx2(unsigned b) {vtx2 = b;}
  void SetGood(char d) {good = d;}
 private:
  unsigned vtx1;
  unsigned vtx2;
  unsigned edgeTy;
  char good;
  unsigned fb_l;
  unsigned fb_r;
};





class CFLGrammar{
 public:
 CFLGrammar(unsigned a, unsigned b, unsigned c, unsigned ep) : LTerm(a), RTerm1(b), RTerm2(c), epsilon(ep) {termNum = 3;}
 CFLGrammar(unsigned a, unsigned b, unsigned ep) : LTerm(a), RTerm1(b), epsilon(ep){termNum = 2;}
  CFLGrammar(){};
  int GetTermNum() {return termNum;}
  int IsEpsilonRule() {if (termNum == 2 && RTerm1 == epsilon) return 1; else return 0; }
  int IsSingleRule() {if (termNum == 2) return 1; else return 0; }
  unsigned GetLTerm(){return LTerm;}
  unsigned GetRTerm1(){return RTerm1;}
  unsigned GetRTerm2(){return RTerm2;}
  void SetLTerm(unsigned a){LTerm = a; termNum++;}
  void SetRTerm1(unsigned a){RTerm1 = a; termNum++;}
  void SetRTerm2(unsigned a){RTerm2 = a; termNum++;}
  void PrintGrammar(){
    cout<<LTerm<<" -> "<<RTerm1;
    if(termNum == 3)
      cout<<" "<<RTerm2;
    cout<<endl;
  }
  


 private:
  int termNum;
  unsigned LTerm;
  unsigned RTerm1;
  unsigned RTerm2;
  unsigned epsilon;
};



class SimpleDotParser{
 public:

  SimpleDotParser() {};
  pair<string, string> ReturnNodePair(string& src, const string& delimiter);

  unsigned BuildNodeMap(stringstream& buffer, unordered_map<string, unsigned>& NodeID,unordered_map<unsigned, string>& NodeID_R);


  unsigned BuildMatrixLin(stringstream& buffer, unordered_map<string, unsigned>& NodeID,  CFLMatrixLin& cm, stack<ReachabilityItemLin>& wl, long** ops, long** cps, long** obs, long** cbs, vector<vector< unordered_set<string>  >>& orig_graph);





  void StripExtra(string& line) {line = line.substr(0, line.find_first_of("["));}
  int IsEdge(const string& line) {if(line.find("->") == string::npos) return 0; else return 1;}

  string GetEdgeLabel(const string& line){
    size_t b,e;
    b = line.find_first_of("\"");
    e = line.find_last_of("\"");
    
    return line.substr(b+1, (e-b-1));
  }
};




inline pair<string, string> SimpleDotParser::ReturnNodePair(string& src, const string& delimiter){
  string from, to;

  StripExtra(src);
  string::size_type delimiterstart = src.find_first_of(delimiter);
  //  istringstream fromstr(src.substr(0, delimiterstart));
  //istringstream tostr(src.substr(delimiterstart + delimiter.size()));

  from = src.substr(0, delimiterstart);
  to = src.substr(delimiterstart + delimiter.size());

  //fromstr>>from;
  //tostr>>to;

  return make_pair(from, to);

  //  string::size_type found = 0, start = 0;
  
  //string substring;

  /*
  do{

    found = src.substr(start).find_first_of(delimiter);
    //cout<<start<<endl;
    if (found == string::npos){
      dst.push_back(src.substr(start));
    }else{
      dst.push_back(src.substr(start, found));
      start = start + found + delimiter.size();
    }
    
    //substring = src.substr(found + 1);
    //cout<<"f " <<found<< " st " <<start<<endl;

    //cout<<"starting "<<start<<"after found = "<<found<<endl;

		  


  }while(found != string::npos);
  
  */
}



inline unsigned SimpleDotParser::BuildNodeMap(stringstream& buffer, unordered_map<string, unsigned>& NodeID, unordered_map<unsigned, string>& NodeID_R){ //return node num.
  string line;
  //ifstream in(infile.c_str());
  if (1){
    


    // operations on the buffer...
    //ifstream in("dd");
  //if (in){
    //stringstream buffer;
    //buffer << in.rdbuf();
  
    while(getline(buffer, line)){
      if(IsEdge(line)){

	string from, to;	
	//EdgeTy edgTy = GetEdgeTy(line);
	//cout<<"line "<< line<< "of type"<<GetEdgeTy(line)<<endl;
	
	pair<string, string> nodes = ReturnNodePair(line, "->");
	from = nodes.first;
	to = nodes.second;
	//cout<<"from "<<from<<" to "<<to<<endl;

	if (NodeID.find(from) == NodeID.end()){ //can't find from
	  unsigned id = NodeID.size();
	  //cout<<from<<" should assign "<<id<<endl;
	  NodeID[from] = id;
	  NodeID_R[id] = from;

	}

	if (NodeID.find(to) == NodeID.end()){ //can't find to
	  unsigned id = NodeID.size();
	  //cout<<to<<" should assign "<<id<<endl;
	  NodeID[to] = id;
	  NodeID_R[id] = to;
	} 
	
      }
    

    }
  }
  return NodeID.size();


}








inline unsigned SimpleDotParser::BuildMatrixLin(stringstream& buffer, unordered_map<string, unsigned>& NodeID, CFLMatrixLin& cm, stack<ReachabilityItemLin>& wl, long** ops, long** cps, long** obs, long** cbs, vector<vector< unordered_set<string>  >>& orig_graph){

  
  string line;
  unsigned edgenum=0;
  //ifstream in(infile.c_str());

  //edgeTy edg;
  while(getline(buffer, line)){
    if(IsEdge(line)){

	string from, to;	
	//build edge map


	string edgelabel = GetEdgeLabel(line);
	//int togrammar=0;

	//int col = stoi(edgelabel);
	
	pair<string, string> hy = ReturnNodePair(edgelabel, "--");
	string ty = hy.first;
	string id = hy.second;





	pair<string, string> nodes = ReturnNodePair(line, "->");
	from = nodes.first;
	to = nodes.second;

	cm.InsertEdge(NodeID[from], NodeID[to]);

	if(ty == "op"){
	  edgenum++;
	  unsigned tmp = q1Lin * numLinEdgeTy + opLin+stoi(id);//stoi(id);

	  ReachabilityItemLin item(NodeID[from], NodeID[to], tmp, 1, NodeID[to], NodeID[from]);
	  wl.push(item);
	  AddItemToSet(ops[NodeID[from]], NodeID[to]);

	  orig_graph[NodeID[from]][NodeID[to]].insert(edgelabel);
	  //AddItemToSet(isst, NodeID[from]);
	}else if(ty == "cp"){
	  edgenum++;
	  unsigned tmp = (opLin+stoi(id)) * numLinEdgeTy + m1Lin;
	  ReachabilityItemLin item(NodeID[from], NodeID[to], tmp, 0,NodeID[to], NodeID[from]);
	  wl.push(item);
	  //AddItemToSet(ised, NodeID[to]);
	  /*	}else if(ty == "cb"){
	  unsigned tmp = obLin * numLinEdgeTy + mLin;
	  ReachabilityItem item(NodeID[from], NodeID[to], tmp);
	  wl.push(item);*/
	  AddItemToSet(cps[NodeID[from]], NodeID[to]);
	  orig_graph[NodeID[from]][NodeID[to]].insert(edgelabel);
	}
	
	else if(ty == "ob"){
	  unsigned tmp = q1Lin * numLinEdgeTy + obLin+stoi(id);
	  ReachabilityItemLin item(NodeID[from], NodeID[to], tmp, 1,NodeID[to], NodeID[from]);
	  wl.push(item);
	  AddItemToSet(obs[NodeID[from]], NodeID[to]);
	  orig_graph[NodeID[from]][NodeID[to]].insert(edgelabel);
	  edgenum++;
	  /*
	  tmp = (obLin+stoi(id)) * numLinEdgeTy + mLin;
	  ReachabilityItemLin item1(NodeID[to], NodeID[from], tmp, 0);
	  wl.push(item1);
	  AddItemToSet(cpbs[NodeID[to]], NodeID[from]);
	  */
	}

	else if(ty == "cb"){
	  edgenum++;
	  unsigned tmp = (obLin+stoi(id)) * numLinEdgeTy + m1Lin;
	  ReachabilityItemLin item1(NodeID[from], NodeID[to], tmp, 0,NodeID[to], NodeID[from]);
	  wl.push(item1);
	  AddItemToSet(cbs[NodeID[from]], NodeID[to]);
	  orig_graph[NodeID[from]][NodeID[to]].insert(edgelabel);
	  /*
	  edgenum++;
	  tmp = q1Lin * numLinEdgeTy + obLin+stoi(id);
	  ReachabilityItemLin item2(NodeID[to], NodeID[from], tmp, 1);
	  wl.push(item2);
	  AddItemToSet(opbs[NodeID[to]], NodeID[from]);
	  */
	  //AddItemToSet(ised, NodeID[to]);
	  
	}


    }
    

    
  }

  return edgenum;
}


//int arrayversion(string Mode, string SS, string TT, unordered_map<string, unsigned>& NodeID,  unordered_map<unsigned, string>& NodeID_R, unsigned NodeNum, stringstream& buffer,   vector<vector< unordered_set<string>  >>& output);
int LCLReach(unordered_map<string, unsigned>& NodeID,  unordered_map<unsigned, string>& NodeID_R, unsigned NodeNum, stringstream& buffer,   vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >>& trace_L,   vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >>& trace_R, CFLMatrixLin& cm1, bitmap** S, vector<vector< unordered_set<string>  >>& orig_graph, long **observed, long **goodq2, bool trace);

void dfs(unsigned Ss, unsigned Tt, unsigned Sum, vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >>& trace_L, vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >>& trace_R, CFLMatrixLin& cm, vector<vector< unordered_set<LinEdgeTy>>>& visited, bitmap** S, long **observed,  vector<vector< unordered_set<string>  >>& orig_graph, unordered_map<unsigned, string>& NodeID_R,vector<vector< unordered_set<string>>>& output);















#endif
