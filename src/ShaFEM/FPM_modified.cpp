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

void FPM_modified::Grow(int *t, int size, int count, bool order) {
    int i, j, *ts;
    Node *par, *me;
    Head *head;
    par = root;

    for (i = (size - 1); (i > 0) && (t[i] >= avglenght); i--) {
        ts = twoset + (avglenght + t[i] - 1) * (t[i] - avglenght) / 2;
        for (j = 0; j < i; j++) ts[t[j]] += count;
    }


    for (i = 0; par->child && (i < size); i++, t++) {
        me = par->child;
        while ((me) && (me->id != *t)) me = me->sibling;

        //if not found
        if (!me) break;

        me->count += count;
        par = me;
    }

    if (i == size) return;

    me = (Node *) fp_buf->newbuf(size - i, sizeof(Node));
//	me = new Node[size-i];

    for (; i < size; i++, t++) {
        //add to first
        head = heads + *t;
        me->init(*t, count, ((par == root) ? 0 : par), 0, par->child, head->next);
        par->child = head->next = me;
        head->nodes++;
        if (head->last == 0) head->last = me;
        par = me;
        me++;
    }
}

int FPM_modified::Build_FP_Tree(ProcessTransactions &data) {
    Transaction *t;
    int trans = 0;

    while (t = data.GetTransaction())
//	while(t = data->GetTransaction())
    {
        Grow(t->items, t->size, t->count);
        trans += t->count;
    }
    return trans;
}

void FPM_modified::UpdateK(int NewPatternNum, int DBSize) {
    int i;

    for (i = 0; i < K_LEVEL; i++) {
        if (DBSize > i * K_STEP) {
            Patterns[i] += NewPatternNum;

        } else break;
    }

    thres_k = 0;

    for (i = K_LEVEL - 1; i > 0; i--) {
        if (Patterns[i - 1] >= 2 * Patterns[i]) {
            thres_k = i * K_STEP;
            break;
        }
    }
}

void FPM_modified::DFP_Tree_Mining(int minsup, OutputData *outfile, int size, int threadid, int threadnum) {
    //for each head list, create a project tree and find the frequent item
    for(int i = 0; i<itemno ; i++)
    {
        //this items is not be mined , used in case of parallel
        //if ( (size ==1) && (i%threadnum) != threadid ) continue;

        Node *n,*p;
        Head* head = heads + i;
        Node* node = head->next;

        //print output
        if (outfile) outfile->write(head->id,head->count,size);

        //print output but not do recursively mine
        if (head->nodes == 1)
        {
            //single path
            if (node->parent && outfile)
            {
                int oldset = outfile->setcount;
                int j = 0;
                for (n = node->parent ; n ; n = n->parent) tmpbuf[j++] = heads[n->id].id;
                outfile->write(tmpbuf,j,0,head->count,size+1);
                UpdateK(outfile->setcount - oldset,1);
            }
            continue;
        }

        int totalcount,itemcount,j,*ts;
        int *t = tmpbuf;
        int	*oldid = tmpbuf;
        int	*newid = tmpbuf + i  ;
        oldid = t;
        totalcount = itemcount = 0;

        //the ts include two halfs: one
        if (i<avglenght)
        {
            ts = tmpbuf + i*2 ;
            memset(ts,0,sizeof(int)*i);
            for (n = node ; n; n = n->next)
                for (p = n->parent; p ; p = p->parent)
                    ts[p->id] += n->count;
        }
        else ts = twoset + (avglenght+i-1)*(i-avglenght)/2;

        for (j=0 ; j < i  ;j++)
        {
            if (ts[j] >= minsup)
            {	totalcount += ts[j];
                oldid[itemcount] = j;
                newid[j] = itemcount++;
            }
            else newid[j] = -1;
        }

        UpdateK(itemcount,head->nodes);

        if (itemcount >1)
        {
            if (head->nodes <= thres_k)
            {
                //same bit size array but save memory
                //double runtime =  clock()/(float)CLOCKS_PER_SEC;
                int tcount = head->nodes/TID_SIZE + ((head->nodes%TID_SIZE)?1:0);
                int MC=0;			//markcount for memory
                unsigned int MR=0;	//markrest for memory
                char* MB;			//markbuf for memory
                MB=fp_buf->bufmark(&MR, &MC);
                int *trans = (int*)fp_buf->newbuf(head->nodes,sizeof(int));
                Item *items = (Item*)fp_buf->newbuf(itemcount,sizeof(Item));
                TID_DATA *tid = (TID_DATA*)fp_buf->newbuf(tcount*itemcount,sizeof(TID_DATA));
                int *sameitems = (int*)fp_buf->newbuf(i,sizeof(int));
                Item *it;
                int h,m;

                memset(tid,0,sizeof(TID_DATA)*tcount*itemcount);

                for (j=0 ; j < itemcount  ;j++)
                {
                    items[j].tid = tid + j*tcount;
                    items[j].id = heads[oldid[j]].id;
                    items[j].count = ts[oldid[j]];
                }

                for (j=0, n = node ; n  ; j++, n = n->next)
                {
                    trans[j] = n->count;
                    h = j / TID_SIZE;
                    m = j% TID_SIZE;

                    for (p = n->parent; p ; p = p->parent)
                    {
                        if (ts[p->id] >= minsup)
                        {
                            it = items + newid[p->id];

                            it->tid[h] |= (0x00000001 << m);
                        }
                    }
                }
                TID_Bit_Vector_Mining(items,itemcount,trans,tcount,minsup,outfile,size + 1,sameitems,0);
                //TIDListRuntime += ( clock()/(float)CLOCKS_PER_SEC - runtime);
                fp_buf->freebuf(MR, MC, MB);
            }
            else
            {
                //UpdateK(itemcount,head->nodes);
                int MC=0;			//markcount for memory
                unsigned int MR=0;	//markrest for memory
                char* MB;			//markbuf for memory
                MB=fp_buf->bufmark(&MR, &MC);
                FPM_modified* ctree = (FPM_modified*)fp_buf->newbuf(1,sizeof(FPM_modified));
                ctree->Create(itemcount,totalcount/head->count,fp_buf,tmpbuf,oneCount,onePos,Patterns);
                for (j=0 ; j < itemcount  ;j++) ctree->heads[j].init(heads[oldid[j]].id,ts[oldid[j]],0,0);

                //for each node this head list i
                for (n = node ; n; n = n->next)
                {
                    //traverse from bottom to top to create a transaction with count n->count
                    for (j = i,p = n->parent; p ; p = p->parent)
                    {
                        if (newid[p->id] != -1) t[--j] = newid[p->id] ;
                    }
                    //create the ctree from the current tree
                    if (j != i) ctree->Grow(t + j, i - j, n->count);
                }

                //then mine it
                ctree->DFP_Tree_Mining(minsup,outfile,size + 1,threadid,threadnum);
                fp_buf->freebuf(MR, MC, MB);
            }
        }
        else if (itemcount == 1 && outfile)
        {
            UpdateK(1,head->nodes);
            outfile->write(heads[oldid[0]].id,ts[oldid[0]],size+1);
        }
    }
}

void FPM_modified::DFP_Tree_Mining_Parallel(int minsup, OutputData *outfile, int size, int threadid, int threadnum) {
    //for each head list, create a project tree and find the frequent item
    //#pragma omp for schedule (dynamic)
#pragma omp for schedule (dynamic) nowait
    //#pragma omp for schedule (static) nowait
    //#pragma omp for
    for(int i = 0; i<itemno ; i++)
    {
        Node *n,*p;
        Head* head = heads + i;
        Node* node = head->next;

        //print output
        if (outfile) outfile->write(head->id,head->count,size);

        //print output but not do recursively mine
        if (head->nodes == 1)
        {
            //single path
            if (node->parent && outfile)
            {
                int oldset = outfile->setcount;
                int j = 0;
                for (n = node->parent ; n ; n = n->parent) tmpbuf[j++] = heads[n->id].id;
                outfile->write(tmpbuf,j,0,head->count,size+1);
                UpdateK(outfile->setcount - oldset,1);
            }


        }
        else
        {

            int totalcount,itemcount,j,*ts;
            int *t = tmpbuf;
            int	*oldid = tmpbuf;
            int	*newid = tmpbuf + i  ;
            oldid = t;
            totalcount = itemcount = 0;

            //the ts include two halfs: one
            if (i<avglenght)
            {
                ts = tmpbuf + i*2 ;
                memset(ts,0,sizeof(int)*i);
                for (n = node ; n; n = n->next)
                    for (p = n->parent; p ; p = p->parent)
                        ts[p->id] += n->count;
            }
            else ts = twoset + (avglenght+i-1)*(i-avglenght)/2;

            for (j=0 ; j < i  ;j++)
            {
                if (ts[j] >= minsup)
                {	totalcount += ts[j];
                    oldid[itemcount] = j;
                    newid[j] = itemcount++;
                }
                else newid[j] = -1;
            }

            UpdateK(itemcount,head->nodes);

            if (itemcount >1)
            {
                if (head->nodes <= thres_k)
                {
                    //same bit size array but save memory
                    //double runtime =  clock()/(float)CLOCKS_PER_SEC;
                    int tcount = head->nodes/TID_SIZE + ((head->nodes%TID_SIZE)?1:0);
                    int MC=0;			//markcount for memory
                    unsigned int MR=0;	//markrest for memory
                    char* MB;			//markbuf for memory
                    MB=fp_buf->bufmark(&MR, &MC);
                    int *trans = (int*)fp_buf->newbuf(head->nodes,sizeof(int));
                    Item *items = (Item*)fp_buf->newbuf(itemcount,sizeof(Item));
                    TID_DATA *tid = (TID_DATA*)fp_buf->newbuf(tcount*itemcount,sizeof(TID_DATA));
                    int *sameitems = (int*)fp_buf->newbuf(i,sizeof(int));
                    Item *it;
                    int h,m;

                    memset(tid,0,sizeof(TID_DATA)*tcount*itemcount);

                    for (j=0 ; j < itemcount  ;j++)
                    {
                        items[j].tid = tid + j*tcount;
                        items[j].id = heads[oldid[j]].id;
                        items[j].count = ts[oldid[j]];
                    }

                    for (j=0, n = node ; n  ; j++, n = n->next)
                    {
                        trans[j] = n->count;
                        h = j / TID_SIZE;
                        m = j% TID_SIZE;

                        for (p = n->parent; p ; p = p->parent)
                        {
                            if (ts[p->id] >= minsup)
                            {
                                it = items + newid[p->id];

                                it->tid[h] |= (0x00000001 << m);
                            }
                        }
                    }
                    TID_Bit_Vector_Mining(items,itemcount,trans,tcount,minsup,outfile,size + 1,sameitems,0);
                    //TIDListRuntime += ( clock()/(float)CLOCKS_PER_SEC - runtime);
                    fp_buf->freebuf(MR, MC, MB);
                }
                else
                {
                    //UpdateK(itemcount,head->nodes);
                    int MC=0;			//markcount for memory
                    unsigned int MR=0;	//markrest for memory
                    char* MB;			//markbuf for memory
                    MB=fp_buf->bufmark(&MR, &MC);
                    FPM_modified* ctree = (FPM_modified*)fp_buf->newbuf(1,sizeof(FPM_modified));
                    ctree->Create(itemcount,totalcount/head->count,fp_buf,tmpbuf,oneCount,onePos,Patterns);
                    for (j=0 ; j < itemcount  ;j++) ctree->heads[j].init(heads[oldid[j]].id,ts[oldid[j]],0,0);

                    //for each node this head list i
                    for (n = node ; n; n = n->next)
                    {
                        //traverse from bottom to top to create a transaction with count n->count
                        for (j = i,p = n->parent; p ; p = p->parent)
                        {
                            if (newid[p->id] != -1) t[--j] = newid[p->id] ;
                        }
                        //create the ctree from the current tree
                        if (j != i) ctree->Grow(t + j, i - j, n->count);
                    }


                    //then mine it
                    ctree->DFP_Tree_Mining(minsup,outfile,size + 1,threadid,threadnum);

                    fp_buf->freebuf(MR, MC, MB);
                }
            }
            else if (itemcount == 1 && outfile)
            {
                UpdateK(1,head->nodes);
                outfile->write(heads[oldid[0]].id,ts[oldid[0]],size+1);
            }
        }
    }
}

// Modified by Evan Stene from the original source to fit the use case of clustering algorithm
void
FPM_modified::Mine_Patterns_Parallel(ProcessTransactions &pt, int minsup, int thres_K, Info *info) {
    //Threshold K
    thres_k = thres_K;
    int threadnum = omp_get_num_threads();
    int threadid = omp_get_thread_num();
    int trans;
    double timecount, timebuild, timemine, timeio, timetotal;

    timetotal = timecount = omp_get_wtime();

    info->pfpm[threadid] = this;

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
    pt.SetDataMask(info->maxitems, &headlist, 0, 1);
    for (int i = 0; i < itemno; i++) heads[i].init(headlist[i].id, headlist[i].count, 0, 0);
    headlist.destroy();

    //Scan database twice to contruct the header table and FP-tree
    Build_FP_Tree(pt);

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

    UpdateK(itemno, K_LEVEL * K_STEP);
    DFP_Tree_Mining_Parallel(minsup, outdata, 1, threadid, threadnum);


#pragma omp atomic
    info->totalitemsets += outdata->setcount;

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