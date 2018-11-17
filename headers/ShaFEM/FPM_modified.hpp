//
// Created by evan on 11/16/18.
//

#ifndef SHAMC_FPM_MODIFIED_HPP
#define SHAMC_FPM_MODIFIED_HPP


#define MAX_ARRAY_SIZE 16*124*1024
#define K_STEP 32
#define K_LEVEL 9
#define LOOKUP_TABLE_SIZE 256
#define MAX_NUM_THREAD 128

#include "../utils/ProcessTransactions.hpp"
#include "buffer.h"

class FPM_modified;

struct Info {
    int *globalcount;
    int totaltrans;
    int totalcount;
    int totalitemsets;
    int maxitems;
    int currentthread;
    FPM_modified	*pfpm[MAX_NUM_THREAD];	//addjust the array if want more current thread
    //OutputData*	out;

    Info() {
        totaltrans = totalcount = totalitemsets = maxitems = currentthread = 0;
        globalcount = 0;
        pfpm = 0;
        //out = 0;
    }

    /* should not have this function, destroy should be invoked by the one that initialized
    ~Info()
    {
        if (out) delete  out;
        if (globalcount) delete[] globalcount;
    }
    */
};

class FPM_modified {
private:
    Head *heads; //list of head nodes
    Node *root; //list of nodes
    int *twoset; //array with 2-lenght itemset frequence
    int itemno;
    int avglenght;
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

    void Grow(int* t, int size,int count,bool order=true);
    int Build_FP_Tree(ProcessTransactions &data);

    void Mine_Patterns_Parallel(ProcessTransactions &pt, int minsup, int thres_K, int methods,
                                Info *info);
};


#endif //SHAMC_FPM_MODIFIED_HPP
