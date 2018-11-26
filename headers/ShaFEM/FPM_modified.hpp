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


/*
 *
 * This code has been modified from the original source written by Lan Vu.
 * It has been changed to work with in-memory transactions from the
 * subspace clustering algorithm, but performs FPGrowth in the same
 * manner as described in the original code.
 *
 */

#ifndef SHAMC_FPM_MODIFIED_HPP
#define SHAMC_FPM_MODIFIED_HPP


#define MAX_ARRAY_SIZE 16*124*1024
#define K_STEP 32
#define K_LEVEL 9
#define TID_SIZE 32
#define LOOKUP_TABLE_SIZE 256
#define MAX_NUM_THREAD 128

#include <cstring>
#include <iostream>
#include "buffer.h"
#include "../utils/ProcessTransactions.hpp"
#include "../utils/SharedSubspace.hpp"

class FPM_modified;

struct Info {
    int *globalcount;
    int totaltrans;
    int totalcount;
    int totalitemsets;
    int maxitems;
    int currentthread;
    FPM_modified *pfpm[MAX_NUM_THREAD];    // adjust the array if want more current thread
    SharedSubspace *subspace;

    Info() {
        totaltrans = totalcount = totalitemsets = maxitems = currentthread = 0;
        globalcount = nullptr;
        //pfpm = nullptr;
        subspace = nullptr;
    }
};

class FPM_modified {
private:
    Head *heads; //list of head nodes
    Node *root; //list of nodes
    int *twoset; //array with 2-lenght itemset frequence
    int itemno;
    int avglength;
    bool istwoset;
    memory *fp_buf;
    int *tmpbuf;
    unsigned int *Patterns;
    int *oneCount;
    int *onePos;
    int thres_k;

public:
    FPM_modified() = default;

    void Initilize(int ItemCount, int AverageLenght, int *OneCountArray, int *OnePosArray);

    bool Create(int ItemCount, int AverageLenght, memory *MemoryBuffer, int *TempBuffer, int *OneCountArray,
                int *OnePosArray, unsigned int *PatternsArray);

    void Grow(int *t, int size, int count, bool order = true);

    int Build_FP_Tree(ProcessTransactions &data);

    void UpdateK(int NewPatternNum, int DBSize);

    int GetCount(const int *trans, TID_DATA tid);

    int Intersect(const Item x, const Item y, Item &xy, int *trans, int size);

    void DFP_Tree_Mining(int minsup, SharedSubspace *subspace, int size = 1, int threadid = 0, int threadnum = 1);

    void DFP_Tree_Mining_Parallel(int minsup, SharedSubspace *subspace, int size, int threadid, int threadnum = 1);


    void
    TID_Bit_Vector_Mining(Item *items, int itemcount, int *trans, int tidsize, int minsup, SharedSubspace *subspace,
                          int size, int *sameitems, int sameitemscount);

    SharedSubspace* Mine_Patterns_Parallel(ProcessTransactions &pt, int minsup, int thres_K, Info *info,  SharedSettings &params);
};


#endif //SHAMC_FPM_MODIFIED_HPP
