/*
   Author:  Lan Vu  

Copyright (c) 2017, University of Colorado Denver All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
   - Neither the name of University of Colorado Denver nor the names of its
       contributors may be used to endorse or promote products derived from
       this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "buffer.h"
#include "DataObject.h"
#include "InputData.h"
#include "OutputData.h"

#define MAX_ARRAY_SIZE 16*124*1024
#define K_STEP 32
#define K_LEVEL 9
#define TID_SIZE 32
#define LOOKUP_TABLE_SIZE 256
#define MAX_NUM_THREAD 128
class FPM;

void BuildTidSizeLookupTable(int *oneCount,int *onePos);

struct Info
{
	int* globalcount;
	int  totaltrans;
	int  totalcount;
	int  totalitemsets;
	int  maxitems;
	int  currentthread;
//	FPM** pfpm; 
	FPM	*pfpm[MAX_NUM_THREAD];	//addjust the array if want more current thread 
	OutputData*	out;
	
	Info()
	{	totaltrans = totalcount =  totalitemsets =  maxitems = currentthread = 0;
		globalcount = 0;
		//pfpm = 0;
		out = 0;
	}

	/* should not have this function, destroy should be invoked by the one that initialized
	~Info()
	{	
		if (out) delete  out;
		if (globalcount) delete[] globalcount;
	}
	*/
};


class FPM
{
public:
	Head* heads; //list of head nodes
	Node* root; //list of nodes
	int* twoset; //array with 2-lenght itemset frequence
	int  itemno;  
	int  avglenght;  
	bool istwoset;	
	memory* fp_buf;
	int* tmpbuf;
	unsigned int* Patterns;
	int* oneCount;
	int* onePos;
	int thres_k;


	void Initilize(int ItemCount,int AverageLenght,int *OneCountArray,int *OnePosArray);
	void Destroy();
	bool Create(int ItemCount,int AverageLenght,memory* MemoryBuffer,int* TempBuffer,int* OneCountArray, int* OnePosArray,unsigned int* PatternsArray);
	
	void Grow(int* t, int size,int count,bool order=true);
	void FP_Tree_Mining(int minsup,OutputData *outfile,int size = 1,int threadid = 0, int threadnum = 1);
	void DFP_Tree_Mining(int minsup,OutputData *outfile,int size = 1,int threadid = 0, int threadnum = 1);
	void DFP_Tree_Mining_Parallel(int minsup,OutputData *outfile,int size,int threadid, int threadnum = 1);
	void LFP_Tree_Mining(int minsup,OutputData *outfile,int size = 1,unsigned int *pPatterns = 0,int threadid = 0, int threadnum = 1);
	void TID_Bit_Vector_Mining(Item *items, int itemcount, int* trans,int tidsize,int minsup,OutputData *outfile,int size, int* sameitems, int sameitemscount);
	void UpdateK(int NewPatternNum,int DBSize);
	void UpdateK_localthreshold(int NewPatternNum,int DBSize,int &k_value,unsigned int *pattern);
	int Intersect(const Item x,const Item y, Item & xy, int* trans,int size);
	int GetCount(const int* trans,TID_DATA tid);

	bool CreateHeaderTable(InputData *indata,int minsup);
	int Build_FP_Tree(InputData *data);
	void Build_FP_Tree_Parallel(InputData *indata,int minsup);
	void Mine_Patterns(char *infile,char *outfile,int minsup,int thres_K,int methods);
	void MergeOutput(char *ScrFileName,char *DesFileName);
	void Mine_Patterns_Parallel(char *infile,char *outfile,int minsup,int thres_K,int methods,Info* info);
	void MergeOutput(char *DesFileName);

	FPM() {heads=0;root=0;twoset=0;}
	~FPM();
	void FEM(char *infile,char *outfile,int minsup,int thres_K);
	void DFEM(char *infile,char *outfile,int minsup);
	void LFEM(char *infile,char *outfile,int minsup);
	
};

class ParFPM
{
	void Mine_Patterns(char *infile,char *outfile,int minsup,int thres_K,int methods);
public:
	void FEM(char *infile,char *outfile,int minsup,int thres_K);
	void DFEM(char *infile,char *outfile,int minsup);
	void LFEM(char *infile,char *outfile,int minsup);
};
