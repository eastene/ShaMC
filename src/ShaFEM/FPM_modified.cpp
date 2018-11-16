//
// Created by evan on 11/16/18.
//

#include <cstring>
#include <iostream>
#include "../../headers/ShaFEM/FPM_modified.hpp"

int OneCount[LOOKUP_TABLE_SIZE];
int OnePos[LOOKUP_TABLE_SIZE];


//initilize the static variable that used
void FPM_modified::Initilize(int ItemCount, int AverageLenght, int *OneCountArray, int *OnePosArray) {
    //initialize the static variables
    fp_buf = new memory(10000, 524288L, 1048576L, 2);
    tmpbuf = (int *) fp_buf->newbuf(ItemCount * 3, sizeof(int));
    Patterns = new unsigned int[K_LEVEL];
    memset(Patterns, 0, sizeof(unsigned int) * K_LEVEL);

    //Create FP-tree data
    Create(ItemCount, AverageLenght, fp_buf, tmpbuf, OneCountArray, OnePosArray, Patterns);

}

bool FPM_modified::Create(int ItemCount, int AverageLenght, memory *MemoryBuffer, int *TempBuffer, int *OneCountArray,
                          int *OnePosArray, unsigned int *PatternsArray) {
    fp_buf = MemoryBuffer;
    tmpbuf = TempBuffer;
    oneCount = OneCountArray;
    onePos = OnePosArray;
    Patterns = PatternsArray;
    itemno = ItemCount;
    avglenght = AverageLenght;

    //create heads
    heads = (Head *) fp_buf->newbuf(ItemCount, sizeof(Head));

    //create root node
    root = (Node *) fp_buf->newbuf(1, sizeof(Node));
    root->init();

    int size = ((ItemCount + AverageLenght - 1) * (ItemCount - AverageLenght)) / 2;

    if ((size > 0) && (size * sizeof(int) < MAX_ARRAY_SIZE)) {
        //create twoset : 2 lenght itemset count
        twoset = (int *) fp_buf->newbuf(size, sizeof(int));
        memset(twoset, 0, sizeof(int) * size);
    } else avglenght = ItemCount;

    return true;
}

// Modified by Evan Stene from the original source to fit the use case of clustering algorithm
void
FPM_modified::Mine_Patterns_Parallel(ProcessTransactions::ProcessTransactions &pt, int minsup, int thres_K, int methods,
                                     Info *info) {
    //Threshold K
    thres_k = thres_K;
    int threadnum = omp_get_num_threads();
    int threadid = omp_get_thread_num();
    int trans;
    double timecount, timebuild, timemine, timeio, timetotal;

    timetotal = timecount = omp_get_wtime();

    //info->pfpm[threadid] = this;

    //Syncrohnize all private count list into an global count list
#pragma omp critical
    {
        if (pt.getCounts().size > info->maxitems) info->maxitems = pt.getCounts().size;
        info->totaltrans += pt.getNumTransactions();
        info->currentthread++;

        if (info->currentthread == threadnum) {
            info->globalcount = new int[info->maxitems];
            memset(info->globalcount, 0, sizeof(int) * (info->maxitems));
            info->currentthread = 0;
        }

    }
#pragma  omp barrier


    for (int i = 0; i < pt.getCounts().size; i++) {
        if (pt.getCounts().counts[i]) {
#pragma omp atomic
            info->globalcount[i] += pt.getCounts().counts[i];
        }
    }
#pragma omp barrier

    //pt.getCounts().destroy();

    //timing
    timecount = omp_get_wtime() - timecount;
    timebuild = omp_get_wtime();

    Headlist headlist;
    int totalcount = 0;
    //Create header table
    for (int i = 0; i < info->maxitems; i++) {
        if (info->globalcount[i] >= minsup) headlist.add(Head(i, info->globalcount[i], 0, 0));
        totalcount += info->globalcount[i];
    }
    itemno = headlist.size;
    headlist.sort();

    //Initilize the tree
    Initilize(itemno, totalcount / info->totaltrans, OneCount, OnePos);

//	#pragma omp barrier
//	root = info->pfpm[0]->root;
//	twoset = info->pfpm[0]->twoset;

    //No item > minsup , exit the program
    if (itemno <= 0) {
        if (threadid == 0) std::cout << "No frequent pattern found\n";
        return;
    }

    //Sort Descendingly the Head Node List
    indata.SetDataMask(info->maxitems, &headlist, 0, 1);
    for (int i = 0; i < itemno; i++) heads[i].init(headlist[i].id, headlist[i].count, 0, 0);
    headlist.destroy();
/*
    //Scan database twice to contruct the header table and FP-tree
    Build_FP_Tree(&indata);
    indata.Close();

    //merge local tree into global tree
#pragma omp barrier
#pragma omp for nowait
    for (int i = 0; i < itemno; i++) {
        for (int j = 1; j < threadnum; j++) {
            if (info->pfpm[j]->heads[i].nodes) {
                if (info->pfpm[0]->heads[i].nodes)
                    info->pfpm[0]->heads[i].last->next = info->pfpm[j]->heads[i].next;
                else
                    info->pfpm[0]->heads[i].next = info->pfpm[j]->heads[i].next;
                info->pfpm[0]->heads[i].last = info->pfpm[j]->heads[i].last;
                info->pfpm[0]->heads[i].nodes += info->pfpm[j]->heads[i].nodes;
            }
        }
    }

    if (twoset) {
        int twosetsize = ((itemno + avglenght - 1) * (itemno - avglenght)) / 2;
        int k = threadid * (twosetsize / threadnum);
        int h = ((threadid + 1) == threadnum) ? twosetsize : (threadid + 1) * (twosetsize / threadnum);

        for (int j = 1; j < threadnum; j++)
            for (int i = k; i < h; i++)
                info->pfpm[0]->twoset[i] += info->pfpm[j]->twoset[i];
    }

    //timing
    timebuild = omp_get_wtime() - timebuild;
    timemine = omp_get_wtime();


    OutputData *outdata = 0;
    if (threadid == 0) {
        outdata = new OutputData(outfile, minsup, itemno);
        info->out = outdata;
    }

#pragma omp barrier
    if (threadid != 0) outdata = new OutputData(info->out->file, minsup, itemno);

    twoset = info->pfpm[0]->twoset;
    heads = info->pfpm[0]->heads;


    switch (methods) {
        case 0:
//				FP_Tree_Mining(minsup,outdata,1,threadid,threadnum);
            break;
        case 1:
            UpdateK(itemno, K_LEVEL * K_STEP);
            DFP_Tree_Mining_Parallel(minsup, outdata, 1, threadid, threadnum);
            break;
        case 2:
//				LFP_Tree_Mining(minsup,outdata,1,0,threadid,threadnum);
            break;
    }

#pragma omp atomic
    info->totalitemsets += outdata->setcount;
*/
    //timing
    timeio = 0;

    //need a better method to syncronize the output writing process
    //if (threadid !=0) outdata->close();

#pragma omp barrier
    timemine = omp_get_wtime() - timemine;
    timetotal = omp_get_wtime() - timetotal;


   /* TODO: make printing optional (debugging?)
    if (threadid == 0) {
        //be careful with this
        //if (outdata->file) fflush(outdata->file);
        //if (outdata->file) fclose(outdata->file);
        std::cout << setiosflags(std::ios::fixed) << std::setprecision(2) << "Minsup: "
             << minsup << "\n"
             << "Number of threads: "
             << threadnum << "\n"
             << "Total frequent patterns: "
             << info->totalitemsets << "\n"
             << "Time: "
             << timetotal << "\n";

    }
    */

    //delete outdata;
    //Destroy();

}
//-----------------------------------------------------------------------------------------
//