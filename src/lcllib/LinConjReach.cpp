#include "CFLReach.h"
#include <iostream>
#include <bitset>
#include <vector>
#include <queue>
#include <stack>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
//#include <unordered_s
#include <sys/time.h>
#include "bitmap.h"
#include <cstring>


using namespace std;

//#define OBMAX 4000000
#define OBMAX ob3133Lin
#define OPMAX op21638Lin
#define Q1OBMAX q1ob3133Lin
#define Q1OPMAX q1op21638Lin

#define Q1JUMP (Q1OPMAX - OPMAX)

//#define VWCOND (lh != q1Lin && (TestItemInSet(ops[v], w) || TestItemInSet(obs[v], w)) )  || (TestItemInSet(cps[v], w) && !TestItemInSet(cbs[v], w) && ((lh>=obLin&&lh<=OBMAX) || (lh>=q1obLin&&lh<=Q1OBMAX)) ) || (TestItemInSet(cbs[v], w) && !TestItemInSet(cps[v], w) && ((lh>=opLin&&lh<=OPMAX) || (lh>=q1opLin&&lh<=Q1OPMAX)) )
//#define WUCOND (rh != m1Lin  && (TestItemInSet(cps[w], u)||TestItemInSet(cbs[w], u)) ) || (rh == m1Lin  && (TestItemInSet(ops[w], u)||TestItemInSet(obs[w], u))) || (TestItemInSet(ops[w], u)&& (rh>=obLin && rh<=OBMAX) ) || (TestItemInSet(obs[w], u)&& (rh>=opLin && rh<=OPMAX) )



#define VWCOND (0)
#define WUCOND (0)

int debug = 0;
int test_trace = 0;
int usearray = 1;
unsigned q2_mLin = q2Lin*numLinEdgeTy + mLin;
unsigned q1_mLin = q1Lin*numLinEdgeTy + mLin;

//should del the following two


//ifstream in("work");
//ifstream in("filelist/pldi2007/linux");
//ifstream in("dotfile/fop");
string version="CFLreach";

//unsigned S_edge = 0;





double elapsed = 0.0;

string ShowSum(unsigned sum){
  string ret;
  if(sum< OPMAX &&  sum >= opLin){
    ret = "op-" + to_string(sum - opLin);
  }else if(sum< OBMAX &&  sum >= obLin){
    ret = "ob-" + to_string(sum - obLin);
  }else if(sum< Q1OPMAX &&  sum >= q1opLin){
    ret = "op-" + to_string(sum - q1opLin);
  }else if(sum< Q1OBMAX &&  sum >= q1obLin){
    ret = "ob-" + to_string(sum - q1obLin);
  }else if(sum == q2Lin){
    ret = "q2";
  }else if(sum == q1Lin){
    ret = "q1";
  }else if(sum == mLin){
    ret = "m";
  }else if(sum == m1Lin){
    ret = "m1";
  }else{
    ret = "unknown!";
  }

  
  return ret;
}



inline unsigned findLinRule(unsigned L, unsigned R){
  unsigned ret=0;
  if(R == q1Lin){
    ret = q1Lin*numLinEdgeTy+L;
  }
  else if(R< OPMAX &&  R >= opLin){
    if(L<  OBMAX && L>= obLin){
      R+= Q1JUMP;
      ret = R*numLinEdgeTy+L;
    }else if(L< OPMAX &&  L >= opLin){
      if(R==L)
	ret = q2Lin*numLinEdgeTy + mLin;
    }else{

      ret = R*numLinEdgeTy+L;
    }
  }
  else if(R == q2Lin){
    if(L< OBMAX && L>= opLin){

      ret = q1Lin*numLinEdgeTy+L;
    }else{
      ret = R*numLinEdgeTy+L;
    }
  }
  else if(R< OBMAX &&  R >= obLin){
    if(L< OPMAX && L>= opLin){
      R+= Q1JUMP;
      ret = R*numLinEdgeTy+L;
    }else if(L< OBMAX &&  L >= obLin){
      if(L==R)
	ret = q2Lin*numLinEdgeTy + mLin;
    }else{
      ret = R*numLinEdgeTy+L;
    }
  }
  else if(R< Q1OBMAX &&  R >= q1obLin){
    if(L< OBMAX &&  L >= obLin){
      if(L == R-Q1JUMP)
	ret = q1Lin*numLinEdgeTy + mLin;
      
    }else{
      ret = R*numLinEdgeTy+L;
    }
  }
  else if(R< Q1OPMAX &&  R >= q1opLin){
    if(L< OPMAX &&  L >= opLin){
      if(L == R-Q1JUMP)
	ret = q1Lin*numLinEdgeTy + mLin;
      
    }else{
      ret = R*numLinEdgeTy+L;
    }
  }

  return ret;

}

//vector<vector< unordered_set<LinEdgeTy>  >> visited(NodeNum);
    
void dfs(unsigned Ss, unsigned Tt, unsigned Sum, vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >>& trace_L, vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >>& trace_R, CFLMatrixLin& cm, vector<vector< unordered_set<LinEdgeTy>>>& visited, bitmap** S, long **observed,  vector<vector< unordered_set<string>  >>& orig_graph, unordered_map<unsigned, string>& NodeID_R,vector<vector< unordered_set<string>>>& output){
 
  //cout<<endl<<"**visiting "<<NodeID_R[Ss]<<" and "<<NodeID_R[Tt]<<endl;


  
  ReachabilityItem item(Ss, Tt, Sum);  

  stack<ReachabilityItem> worklist;
  worklist.push(item);

  while(!worklist.empty()){
    unsigned NodeS = worklist.top().GetVtx1();
    unsigned NodeT = worklist.top().GetVtx2();
    unsigned Summary = worklist.top().GetEdgeTy();

    worklist.pop();
      
    visited[NodeS][NodeT].insert((LinEdgeTy) Summary);  

    //case L
    vector<unsigned> *out=cm.RetOutvec(NodeS);

    //foreach q in R[s][t]
    for (LinEdgeTy x : trace_R[NodeS][NodeT][(LinEdgeTy) Summary]){
    

      for(vector<unsigned>::iterator iter= out->begin(); iter!= out->end(); iter++){

	unsigned w = *iter;

	//cout<<"w is"<<NodeID_R[w]<<endl;
	//cout<<"doing "<<NodeID_R[w]<<" cond: "<<TestItemInSet(observed[w], NodeT)<<endl;
	if( TestItemInSet(observed[w], NodeT) && !bitmap_empty_p(S[w][NodeT])){
	  //cout<<"checking1 "<<NodeID_R[w]<<endl;
	  bitmap_iterator bi;
	  unsigned oo;
	  //foreach summary S[w][t]
	  EXECUTE_IF_SET_IN_BITMAP(S[w][NodeT], 0, oo, bi){
	    //get q (left part)

	    unsigned lh = oo /  numLinEdgeTy;

	    if(lh == x){

	      if(output[NodeS][w].size() < 1){
		for(string x : orig_graph[NodeS][w]){
		  //12676->12654[label="cb--14"]
		  //cout<<"INFO: "<<NodeID_R[NodeS]<<"->"<<NodeID_R[w]<<"[label=\""<<x<<"\"]"<<endl;
		  output[NodeS][w].insert(x);		  
		}

	      }
	    
	      //todo: print NodeS->w with label
	      if(visited[w][NodeT].find((LinEdgeTy) oo) == visited[w][NodeT].end()){ // need to consider summary

		ReachabilityItem item(w, NodeT, oo);  
		worklist.push(item);
	      }

	    }
	  }
	}else if(	!TestItemInSet(observed[w], NodeT) || w == NodeT ||  (TestItemInSet(observed[w], NodeT) && bitmap_empty_p(S[w][NodeT]))){
	  //cout<<"checking "<<NodeID_R[w]<<endl;
	  if(orig_graph[w][NodeT].size() > 0 || w == NodeT){ // it means that w->nodeT is the last edge.
	    if(output[NodeS][w].size() < 1 ){
	      for(string x : orig_graph[NodeS][w]){
		//12676->12654[label="cb--14"]
		//cout<<"INFO: "<<NodeID_R[NodeS]<<"->"<<NodeID_R[w]<<"[label=\""<<x<<"\"]"<<endl;
		output[NodeS][w].insert(x);
	      }

	    }
	    if(output[w][NodeT].size() < 1){
	      for(string x : orig_graph[w][NodeT]){
		//12676->12654[label="cb--14"]
		//cout<<"INFO: "<<NodeID_R[w]<<"->"<<NodeID_R[NodeT]<<"[label=\""<<x<<"\"]"<<endl;
		output[w][NodeT].insert(x);
	      }

	    }
	  }
	  //	cout<<"s-Shuo: "<<NodeS<<" -> "<<w<<endl;
	  //cout<<"s-Shuo: "<<w<<" -> "<<NodeT<<endl;
	}  
  
      }
    }


    //case R
    vector<unsigned> *in=cm.RetInvec(NodeT);

    //foreach Z in L[s][t]
    for (LinEdgeTy x : trace_L[NodeS][NodeT][(LinEdgeTy) Summary]){


      for(vector<unsigned>::iterator iter= in->begin(); iter!= in->end(); iter++){

	unsigned w = *iter;

	//      cout<<"w is"<<NodeID_R[w]<<endl;
	if( TestItemInSet(observed[NodeS], w) && !bitmap_empty_p(S[NodeS][w])){

	  bitmap_iterator bi;
	  unsigned oo;
	  //foreach summary S[s][w]
	  EXECUTE_IF_SET_IN_BITMAP(S[NodeS][w], 0, oo, bi){
	    //get Z (right part)

	    unsigned rh = oo %  numLinEdgeTy; 
	    if(rh == x){
	      //cout<<"hit-good "<<endl;
	      if(output[w][NodeT].size() < 1){
		for(string x : orig_graph[w][NodeT]){
		  //12676->12654[label="cb--14"]
		  //cout<<"INFO: "<<NodeID_R[w]<<"->"<<NodeID_R[NodeT]<<"[label=\""<<x<<"\"]"<<endl;
		  output[w][NodeT].insert(x);
		}

	      }
	    


	      //todo: print NodeS->w with label
	      if(visited[NodeS][w].find((LinEdgeTy) oo) == visited[NodeS][w].end()){ // need to consider summary
		ReachabilityItem item(NodeS, w, oo);  
		worklist.push(item);


	      }   
	    }

	  }
	}else if(	!TestItemInSet(observed[NodeS], w) || w == NodeS || (TestItemInSet(observed[NodeS], w) && bitmap_empty_p(S[NodeS][w]))){
	  //cout<<"hit1"<<endl;

	  if(orig_graph[NodeS][w].size() > 0 || w == NodeS){ // it means that w->nodeT is the last edge.
	    if(output[NodeS][w].size() < 1){
	      for(string x : orig_graph[NodeS][w]){
		//12676->12654[label="cb--14"]

		//cout<<"INFO: "<<NodeID_R[NodeS]<<"->"<<NodeID_R[w]<<"[label=\""<<x<<"\"]"<<endl;
		output[NodeS][w].insert(x);
	      }

	    }
	    if(output[w][NodeT].size() < 1){
	      for(string x : orig_graph[w][NodeT]){
		//12676->12654[label="cb--14"]

		//cout<<"INFO: "<<NodeID_R[w]<<"->"<<NodeID_R[NodeT]<<"[label=\""<<x<<"\"]"<<endl;
		output[w][NodeT].insert(x);
	      }

	    }
	  }
	  //	cout<<"s-Shuo: "<<NodeS<<" -> "<<w<<endl;
	  //cout<<"s-Shuo: "<<w<<" -> "<<NodeT<<endl;
	}  
  
      }
    }

  }


}










int LCLReach(unordered_map<string, unsigned>& NodeID,  unordered_map<unsigned, string>& NodeID_R, unsigned NodeNum, stringstream& buffer, vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >>& trace_L,   vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >>& trace_R, CFLMatrixLin& cm1, bitmap** S, vector<vector< unordered_set<string>  >>& orig_graph, long **observed, long **goodq2, bool trace){
  



  /*
  unordered_map<unsigned, unsigned> LinRule;
  //should init linrules.
  LinRule[opLin * numLinEdgeTy + q1Lin] = q1Lin*numLinEdgeTy + opLin;
  LinRule[obLin * numLinEdgeTy + q1Lin] = q1Lin*numLinEdgeTy + obLin;
  LinRule[mLin * numLinEdgeTy + q1Lin] = q1Lin*numLinEdgeTy + mLin;


  LinRule[opLin * numLinEdgeTy + opLin] = q2Lin*numLinEdgeTy + mLin;
  LinRule[obLin * numLinEdgeTy + opLin] = q1opLin*numLinEdgeTy + obLin;
  LinRule[mLin * numLinEdgeTy + opLin] = opLin*numLinEdgeTy + mLin;


  LinRule[mLin * numLinEdgeTy + q2Lin] = q2Lin*numLinEdgeTy + mLin;
  LinRule[opLin * numLinEdgeTy + q2Lin] = q1Lin*numLinEdgeTy + opLin;
  LinRule[obLin * numLinEdgeTy + q2Lin] = q1Lin*numLinEdgeTy + obLin;


  LinRule[opLin * numLinEdgeTy + obLin] = q1obLin*numLinEdgeTy + opLin;
  LinRule[mLin * numLinEdgeTy + obLin] = obLin*numLinEdgeTy + mLin;
  LinRule[obLin * numLinEdgeTy + obLin] = q2Lin*numLinEdgeTy + mLin;

  LinRule[opLin * numLinEdgeTy + q1obLin] = q1obLin*numLinEdgeTy + opLin;
  LinRule[mLin * numLinEdgeTy + q1obLin] = q1obLin*numLinEdgeTy + mLin;
  LinRule[obLin * numLinEdgeTy + q1obLin] = q1Lin*numLinEdgeTy + mLin;

  LinRule[opLin * numLinEdgeTy + q1opLin] = q1Lin*numLinEdgeTy + mLin;
  LinRule[mLin * numLinEdgeTy + q1opLin] = q1opLin*numLinEdgeTy + mLin;
  LinRule[obLin * numLinEdgeTy + q1opLin] = q1opLin*numLinEdgeTy + obLin;

  */



SimpleDotParser dotparser;

    struct timeval begin1, end1;


    





    //cout<<"doing "<<line<<" of size "<<NodeNum<<endl;
    //NodeNum = dotparser.BuildMatrix(line, NodeID);

    //CFLBitTable bt(NodeNum);
    //cout<<"node "<<NodeNum<<endl;
    //CFLMatrix cm(NodeNum);
    //unsigned long abc = 100000;
    //cout<<"lala" << abc*abc<<endl;
    //CFLMatrix cm(NodeNum);
    //CFLMatrixLin cm1(NodeNum);
    stack<ReachabilityItemLin> worklist;
    unsigned WordsInSet = NodeNum/64+1;
    /*
    isst = (long *)malloc(sizeof(long)*NodeNum);
    ised = (long *)malloc(sizeof(long)*NodeNum);
    memset(isst, 0, sizeof(long) * WordsInSet);
    memset(ised, 0, sizeof(long) * WordsInSet);
    */	  
    long **ops = (long **)malloc(sizeof(long *)*NodeNum);
    long **cps = (long **)malloc(sizeof(long *)*NodeNum);
    long **obs = (long **)malloc(sizeof(long *)*NodeNum);
    long **cbs = (long **)malloc(sizeof(long *)*NodeNum);



    //add trace

    //vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >> trace_L(NodeNum);
    //vector<vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>  >> trace_R(NodeNum);

if (trace) {
    for (unsigned i = 0; i < NodeNum; i++) { 
      trace_L[i] = vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>   >(NodeNum);
      trace_R[i] = vector< unordered_map<LinEdgeTy, unordered_set<LinEdgeTy>>   >(NodeNum);

      //      cout<<"we are done"<<i<<" of "<<NodeNum<<endl;      
    } 
}


    for (unsigned i = 0; i < NodeNum; i++) {
      orig_graph[i] = vector< unordered_set<string>   >(NodeNum);
    }
    


    //return 1;






    

    for (unsigned ii=0;ii<NodeNum; ii++){

      ops[ii] = (long *)malloc(sizeof(long)*WordsInSet);
      memset(ops[ii], 0, sizeof(long) * WordsInSet);
      cps[ii] = (long *)malloc(sizeof(long)*WordsInSet);
      memset(cps[ii], 0, sizeof(long) * WordsInSet);
      obs[ii] = (long *)malloc(sizeof(long)*WordsInSet);
      memset(obs[ii], 0, sizeof(long) * WordsInSet);
      cbs[ii] = (long *)malloc(sizeof(long)*WordsInSet);
      memset(cbs[ii], 0, sizeof(long) * WordsInSet);

    }

    buffer.clear();
    buffer.seekg(0);
    unsigned EdgeNum = dotparser.BuildMatrixLin(buffer, NodeID, cm1, worklist, ops, cps, obs, cbs, orig_graph);


    /*
    for(int ii=0; ii<NodeNum; ii++){
    vector<NodeWithColor>* out1 = cm1.RetOutvec(ii);

    for(vector<NodeWithColor>::iterator iter = out1->begin(); iter!=out1->end(); iter++){
      cout<<ii<<" "<< iter->node<<" color "<<iter->color<<endl;

    }
    }*/

    //NodeID.clear();
    //cout<<"all nodes: "<<NodeNum<<" edges: "<<EdgeNum<<endl;
    


    gettimeofday(&begin1, NULL);
    
    
    //init data structure for linearConj
    

    //u*NodeNum+v


    //bitmap *bt = new bitmap[1000000];

    


    long **goodq1 = (long **)malloc(sizeof(long *)*NodeNum);


    bitmap* L[NodeNum];
    bitmap* R[NodeNum];

    bitmap* L_b[NodeNum];


    for (unsigned ii=0;ii<NodeNum; ii++){

      L[ii] = new bitmap[NodeNum];
      R[ii] = new bitmap[NodeNum];
      S[ii] = new bitmap[NodeNum];
      L_b[ii] = new bitmap[NodeNum];
      


      observed[ii] = (long *)malloc(sizeof(long)*WordsInSet);
      memset(observed[ii], 0, sizeof(long) * WordsInSet);
      goodq2[ii] = (long *)malloc(sizeof(long)*WordsInSet);
      memset(goodq2[ii], 0, sizeof(long) * WordsInSet);
      goodq1[ii] = (long *)malloc(sizeof(long)*WordsInSet);
      memset(goodq1[ii], 0, sizeof(long) * WordsInSet);

    }

    //vector<bitmap> aa(4000000);
    //char observed[2000][2000] = {0};
    //bitset<OBMAX> observed;




    /*
    for(unsigned ii=0;ii<NodeNum;ii++){
      for(unsigned jj=0; jj<NodeNum; jj++){
	
	AddItemToSet(observed[ii], jj);
      }
    }
    for(unsigned ii=0;ii<NodeNum;ii++){
      for(unsigned jj=0; jj<NodeNum; jj++){
	
	cout<<TestItemInSet(observed[ii], jj)<<endl;;
      }
    }



    cout<<"in set "<<TestItemInSet(observed[23], 333)<<endl;

    cout<<"ss"<<endl;*/
    //long * observed  = (long *)malloc(sizeof(long)*WordsInSet);
    //MEM_USAGE();     
    /*
   for (unsigned ii=0;ii<2000; ii++){
     for (unsigned jj=0;jj<2000; jj++){
       observed[ii][jj] = 0;
     }
    }
    */

    //bitmap observed= BITMAP_ALLOC(NULL);

    
    //unordered_map< unsigned, unordered_map<unsigned,char> > S;   //node pair -> se pair

    //main algo


    // insert singlton rule A -> a
    /*
    for(unsigned i = 0; i< NodeNum; i++){
      vector<unsigned>* out = cm1.RetOutvec(i); 
      for(vector<unsigned>::iterator iter=out->begin(); iter!=out->end(); iter++){



	  ReachabilityItemLin t(i, *iter, iter->color);
	  worklist.push(t);
	  

      }
      
      }*/




    

    while(!worklist.empty()){

      unsigned fbl = worklist.top().GetFBL();
      unsigned fbr = worklist.top().GetFBR();
      
      unsigned u = worklist.top().GetVtx1();
      unsigned v = worklist.top().GetVtx2();
      unsigned X_sep = worklist.top().GetEdgeTy();
      char good = worklist.top().IsGood();
      unsigned L_ser = X_sep % numLinEdgeTy;
      unsigned R_sel = X_sep / numLinEdgeTy;

      worklist.pop();
      vector<unsigned> *out=cm1.RetOutvec(v);
      if(debug) cout<<"POP: "<<NodeID_R[u]<<" "<<NodeID_R[v]<<" sum: "<<ShowSum(R_sel)<<" "<<ShowSum(L_ser)<<endl;


      if(u == v && R_sel == q2Lin && L_ser == mLin){
	if (debug) cout<<"skip"<<endl;
	continue;
      }

      
      for(vector<unsigned>::iterator iter= out->begin(); iter!= out->end(); iter++){
	unsigned w = *iter;
#ifdef ALIAS
	if(w == fbr) continue;
#endif

	if(!TestItemInSet(observed[u], w)){

	  L[u][w] = BITMAP_ALLOC(NULL);
	  R[u][w] = BITMAP_ALLOC(NULL);
	  S[u][w] = BITMAP_ALLOC(NULL);
	  L_b[u][w] = BITMAP_ALLOC(NULL);
	  AddItemToSet(observed[u], w);
	  //cout<<"observed added "<<NodeID_R[u]<<" "<<NodeID_R[w]<<endl;
	}


	//cout<<"finding "<<NodeID_R[w]<<endl;
	if (bitmap_bit_p(L_b[u][w], L_ser) == 0 && bitmap_bit_p(L[u][w], L_ser) == 0){  //new summary
	  int newgood=0;
	  if(good ==0 || (R_sel != q1Lin && R_sel != q2Lin)){ //bad
	    //cout<<"add l_b "<<NodeID_R[u]<<" "<<NodeID_R[w]<<" sum "<<ShowSum(L_ser)<<endl;
	    bitmap_set_bit(L_b[u][w], L_ser);
	  }else{
	    bitmap_set_bit(L[u][w], L_ser);
	    newgood=1;
	  }


	  if(!bitmap_empty_p(R[u][w])){
	    
	    bitmap_iterator bi;
	    unsigned oo;

	    EXECUTE_IF_SET_IN_BITMAP(R[u][w], 0, oo, bi){

	      //cout<<"doing lh "<<ShowSum(L_ser)<<" rh "<<ShowSum(oo)<<endl;		
	      unsigned lin_sep = findLinRule(L_ser, oo);

	      if(lin_sep){

		unsigned rh = lin_sep % numLinEdgeTy;
		unsigned lh = lin_sep / numLinEdgeTy;

		//cout<<NodeID_R[u]<<" "<<NodeID_R[w]<<" with lh"<<ShowSum(lh)<<" rh"<<ShowSum(rh)<<endl;		
		if(!(VWCOND )){

		      //add trace
if (trace) {
		      trace_L[u][w][(LinEdgeTy) lin_sep].insert((LinEdgeTy) L_ser);
		      trace_R[u][w][(LinEdgeTy) lin_sep].insert((LinEdgeTy) oo);
		      if (test_trace) cout<<"tracing: "<<NodeID_R[u]<<" and "<<NodeID_R[w]<<" added "<<ShowSum(L_ser)<<" "<<ShowSum(oo)<<" due to "<<ShowSum(X_sep)<<endl;
		      if (test_trace)  cout<<"Inserting summary: "<<NodeID_R[u]<<" and "<<NodeID_R[w]<<" added "<<ShowSum(lin_sep)<<endl;
}		  
		  if(bitmap_bit_p(S[u][w], lin_sep) == 0  || (newgood && lh == q1Lin && !TestItemInSet(goodq1[u], w)) || (newgood && lh == q2Lin && !TestItemInSet(goodq2[u], w))){
		      bitmap_set_bit(S[u][w], lin_sep);

		      
		  
		      ReachabilityItemLin item(u, w, lin_sep, 0, fbl, v);
		      if(bitmap_bit_p(L[u][w], L_ser)){
			item.SetGood(1);
		    
			if(lin_sep == q2_mLin){
			  AddItemToSet(goodq2[u], w);
			  if(debug) cout<<"-add goodq2 2 "<<NodeID_R[u]<<" "<<NodeID_R[w]<<endl;
			}
			else if(lin_sep == q1_mLin)
			  AddItemToSet(goodq1[u], w);


		      }
		      if(debug) cout<<"      ADD1: "<<NodeID_R[u]<<" "<<NodeID_R[w]<<" sum: "<<ShowSum(lh)<<" "<<ShowSum(rh)<<endl;
		      worklist.push(item);
		     }
		    //}
		}
	      } 
	    }
	    //}
	  }
	}else if(good && bitmap_bit_p(L_b[u][w], L_ser)  &&(R_sel == q1Lin || R_sel == q2Lin) ){
	  bitmap_clear_bit(L_b[u][w], L_ser);
	  bitmap_set_bit(L[u][w], L_ser);

	  if(!bitmap_empty_p(R[u][w])){
	    
	    
	    bitmap_iterator bi;
	    unsigned oo;
	    
	    
	    EXECUTE_IF_SET_IN_BITMAP(R[u][w], 0, oo, bi){
	      

	      unsigned lin_sep = findLinRule(L_ser, oo);
	      
	      
	      if(lin_sep){
		//if(!(lin_sep == q2_mLin && u==w)){
		unsigned rh = lin_sep % numLinEdgeTy;
		unsigned lh = lin_sep / numLinEdgeTy;

		//if(1){
		if(!( VWCOND)){
		

		  if(lin_sep == q2_mLin){
		  
		    AddItemToSet(goodq2[u], w);
		    if(debug) cout<<"-add goodq2 3 "<<NodeID_R[u]<<" "<<NodeID_R[w]<<endl;
		  
		  }else if(lin_sep == q1_mLin){
		  
		    AddItemToSet(goodq1[u], w);
		  
		  }
	
		  bitmap_set_bit(S[u][w], lin_sep);

if (trace) {
		  //add trace
		  trace_L[u][w][(LinEdgeTy) lin_sep].insert((LinEdgeTy) L_ser);
		  trace_R[u][w][(LinEdgeTy) lin_sep].insert((LinEdgeTy) oo);
		  if (test_trace) cout<<"tracing: "<<NodeID_R[u]<<" and "<<NodeID_R[w]<<" added "<<ShowSum(L_ser)<<" "<<ShowSum(oo)<<" due to "<<ShowSum(X_sep)<<endl;
		  if (test_trace) cout<<"Inserting summary: "<<NodeID_R[u]<<" and "<<NodeID_R[w]<<" added "<<ShowSum(lin_sep)<<endl;
}
		  ReachabilityItemLin item(u, w, lin_sep, 1, fbl, v);
		  worklist.push(item);
		  if(debug) cout<<"      ADD2: "<<NodeID_R[u]<<" "<<NodeID_R[w]<<" sum: "<<ShowSum(lh)<<" "<<ShowSum(rh)<<endl;
		  //}
		}
	      }
	    }
	    //}
	  }
	  
	  
	  
	}
      }







    
    
    
      
      vector<unsigned> *in=cm1.RetInvec(u);
      

      for(vector<unsigned>::iterator iter= in->begin(); iter!= in->end(); iter++){

	unsigned w = *iter;

#ifdef ALIAS
	if(w == fbl) continue;
#endif
	if(!TestItemInSet(observed[w], v)){
	  
	  L[w][v] = BITMAP_ALLOC(NULL);
	  R[w][v] = BITMAP_ALLOC(NULL);
	  S[w][v] = BITMAP_ALLOC(NULL);
	  L_b[w][v] = BITMAP_ALLOC(NULL);


	  AddItemToSet(observed[w], v);
	  //cout<<"observed added "<<NodeID_R[w]<<" "<<NodeID_R[v]<<endl;
	  
	}


	if (bitmap_bit_p(R[w][v], R_sel) == 0){
	  //cout<<"ddd "<<NodeID_R[w]<<endl;
	  //if( ! (L_ser == mLin &&  (TestItemInSet(cps[w], u)||TestItemInSet(cbs[w], u))) ){ //an invalid case.
	    bitmap_set_bit(R[w][v], R_sel);
	    //cout<<"set R "<<NodeID_R[w]<<" "<<NodeID_R[v]<<" sum: "<<ShowSum(R_sel)<<endl;
	    //}




	  
	  if(!bitmap_empty_p(L_b[w][v])){
	    bitmap_iterator bi;
	    unsigned oo;

	    EXECUTE_IF_SET_IN_BITMAP(L_b[w][v], 0, oo, bi){


	      unsigned lin_sep = findLinRule(oo, R_sel);
	      if(lin_sep){
		unsigned rh = lin_sep % numLinEdgeTy;
		unsigned lh = lin_sep / numLinEdgeTy;
		if(debug) cout<<"LB using node "<<NodeID_R[w]<<ShowSum(oo)<<" "<<ShowSum(R_sel)<<" return "<<ShowSum(lin_sep)<<endl;
	      
		if(!(WUCOND )){
if (trace) {
		    //trace
		    trace_L[w][v][(LinEdgeTy) lin_sep].insert((LinEdgeTy) oo);
		    trace_R[w][v][(LinEdgeTy) lin_sep].insert((LinEdgeTy) R_sel);
		  
		    if (test_trace) cout<<"tracing: "<<NodeID_R[w]<<" and "<<NodeID_R[v]<<" added "<<ShowSum(oo)<<" "<<ShowSum(R_sel)<<endl;
		    if (test_trace) cout<<"Inserting summary: "<<NodeID_R[w]<<" and "<<NodeID_R[v]<<" added "<<ShowSum(lin_sep)<<endl;
}		
		  if (bitmap_bit_p(S[w][v], lin_sep) == 0){
		    
		    bitmap_set_bit(S[w][v], lin_sep);

		    
		    ReachabilityItemLin item(w, v, lin_sep,0, u, fbr);
		    worklist.push(item);
		    
		    if(debug) cout<<"      ADD3: "<<NodeID_R[w]<<" "<<NodeID_R[v]<<" sum: "<<ShowSum(lh)<<" "<<ShowSum(rh)<<endl;
		    
		  }
		}
	      } //}
	    }
	  }
	  
	  if(!bitmap_empty_p(L[w][v])){
	    bitmap_iterator bi;
	    unsigned oo;
	  
	    EXECUTE_IF_SET_IN_BITMAP(L[w][v], 0, oo, bi){

	      unsigned lin_sep = findLinRule(oo, R_sel);
	      if(debug) cout<<"L using node "<<NodeID_R[w]<<ShowSum(oo)<<" "<<ShowSum(R_sel)<<" return "<<ShowSum(lin_sep)<<endl;

	      if(lin_sep){
		//cout<<"hit"<<endl;
		unsigned rh = lin_sep % numLinEdgeTy;
		unsigned lh = lin_sep / numLinEdgeTy;
		if(!( WUCOND)){
		  //cout<<"hit1"<<endl;
		  if (trace) {
		    //trace
		    trace_L[w][v][(LinEdgeTy) lin_sep].insert((LinEdgeTy) oo);
		    trace_R[w][v][(LinEdgeTy) lin_sep].insert((LinEdgeTy) R_sel);
		    if (test_trace) cout<<"tracing: "<<NodeID_R[w]<<" and "<<NodeID_R[v]<<" added "<<ShowSum(oo)<<" "<<ShowSum(R_sel)<<endl;
		    if (test_trace) cout<<"Inserting summary: "<<NodeID_R[w]<<" and "<<NodeID_R[v]<<" added "<<ShowSum(lin_sep)<<endl;
		  }

		  if (lin_sep ==q2_mLin || lin_sep == q1_mLin){
		    
		    if(lin_sep == q2_mLin && !TestItemInSet(goodq2[w], v)){

		      if(debug) cout<<"-add goodq2 1 "<<NodeID_R[w]<<" "<<NodeID_R[v]<<endl;
		      AddItemToSet(goodq2[w], v);
		      
		      ReachabilityItemLin item(w, v, lin_sep, 1, u, fbr);
		      bitmap_set_bit(S[w][v], lin_sep);
		      

		      worklist.push(item);
		      if(debug) cout<<"      ADD6: "<<NodeID_R[w]<<" "<<NodeID_R[v]<<" sum: "<<ShowSum(lh)<<" "<<ShowSum(rh)<<endl;
		      
		    }else if (lin_sep == q1_mLin && !TestItemInSet(goodq1[w], v)){
		      
		      AddItemToSet(goodq1[w], v);
		      bitmap_set_bit(S[w][v], lin_sep);

		      ReachabilityItemLin item(w, v, lin_sep, 1,u, fbr);
		      worklist.push(item);
		      if(debug) cout<<"      ADD4: "<<NodeID_R[w]<<" "<<NodeID_R[v]<<" sum: "<<ShowSum(lh)<<" "<<ShowSum(rh)<<endl;
		      
		    }

		  }      
		  else if (bitmap_bit_p(S[w][v], lin_sep) == 0){
		  
		    bitmap_set_bit(S[w][v], lin_sep);

		    ReachabilityItemLin item(w, v, lin_sep, 1,u, fbr);
		    worklist.push(item);
		    if(debug) cout<<"      ADD5: "<<NodeID_R[w]<<" "<<NodeID_R[v]<<" sum: "<<ShowSum(lh)<<" "<<ShowSum(rh)<<endl;
		    
		  }
		
		}	//}
	      }
	    }
	  }
	}
      }
 

      














    }
    
    gettimeofday(&end1, NULL);
    elapsed += ((end1.tv_sec - begin1.tv_sec) + ((end1.tv_usec - begin1.tv_usec) / 1000000.0));    

    //    unsigned  tar = (q2Lin * numLinEdgeTy + mLin);
    unsigned spairs=0;

    for(unsigned ii=0;ii<NodeNum;ii++){
      for(unsigned jj=0; jj<NodeNum; jj++){
	if(ii==jj){
	   spairs++;
	   //if(Mode.compare("1") == 0) cout<<"ALLPAIRS: "<<NodeID_R[ii]<<" "<<NodeID_R[ii]<<endl;
	}
	else{
	  if( TestItemInSet(observed[ii],jj) && bitmap_bit_p(S[ii][jj], q2_mLin))
	    if(TestItemInSet(goodq2[ii], jj)){
	      //if(TestItemInSet(isst, ii) && TestItemInSet(ised, jj))
	      spairs++;
	      //if(debug) cout<<"got ii "<<NodeID_R[ii]<<" jj "<<NodeID_R[jj]<<endl;
	      //if(Mode.compare("1") == 0) cout<<"ALLPAIRS: "<<NodeID_R[ii]<<" "<<NodeID_R[jj]<<endl;
	    }
	}
      }
    }

    //cout<<"Result s pairs: "<<spairs<<endl;





    
    //cout<<"Result s pairs: "<<NS.size()<<endl;
     //MEM_USAGE();     


     //  }








    //arrayreach(cm1, CGVec);
    
    


 


    

  //SimpleDotParser dotparser;
  //dotparser.ParsingFile("./data/CINT2000/254.gap/plist.o.lala.dot");
  
  //cout<<"Runtime: "<<elapsed<<endl;





  return 0;
  

}


/*
int main(int argc, char* argv[]){

  if(argc != 4){
    cout<<"Usage: ./LinConjReach Mode [NodeS] [NodeT]"<<endl;
    cout<<"Mode 1: printing all reachable pairs"<<endl;
    cout<<"Mode 2: printing all contributing edges for NodeS and NodeT"<<endl;
    cout<<"Mode 3: printing all contributing edges"<<endl;
    abort();
  }



  cout<<version<<endl;
  string line;
  unsigned NodeNum;
  SimpleDotParser dotparser;
  unordered_map<string, unsigned> NodeID;
  unordered_map<unsigned, string> NodeID_R;
  stringstream buffer;   //For OLD version: see also "in" defined in CFLReach.h

  //ifstream in1("scipiex.dot");
  //buffer << in1.rdbuf();
  buffer << "24520->1[label=\"op--2\"]\n";
  buffer << "1->10108[label=\"cp--2\"]\n";

    
  NodeNum = dotparser.BuildNodeMap(buffer, NodeID, NodeID_R);
  vector<vector< unordered_set<string>  >> output(NodeNum);
  for (unsigned i = 0; i < NodeNum; i++) {
    output[i] = vector< unordered_set<string>   >(NodeNum);
  }

    
  arrayversion(argv[1], argv[2],argv[3], NodeID, NodeID_R, NodeNum, buffer, output);


  for(unsigned i = 0; i < NodeNum; i++){
    for(unsigned j = 0; j < NodeNum; j++){
      if(output[i][j].size() > 0){
	for(string x : output[i][j] )
	  cout<<"INFO: "<<NodeID_R[i]<<"->"<<NodeID_R[j]<<"[label=\""<<x<<"\"]"<<endl;
      }
    }
  }
  return 0;
}
*/
