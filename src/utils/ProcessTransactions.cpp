//
// Created by evan on 11/16/18.
//

#include <cmath>

#include "../../headers/utils/ProcessTransactions.hpp"

typedef uint16_t DimensionItem;

void ProcessTransactions::buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me) {
    Row *centroid = X.getRow(centroidID);
    Row *point;

    for (uint64_t i = 0; i < X.getPartitionSize(me); i++) {
        if (i == centroid->idx)
            continue;

        point = X.getRowFromPartition(i, me);
        for (DimensionItem j = 0; j < point->cells.size(); j++) {
            if (fabs(point->cells[j] - centroid->cells[j]) <= X.getSettings().width) {
                this->transactions[i].emplace_back(j);
                this->counts.count(j);
            }
        }
        numTransactions++;
    }
}

void ProcessTransactions::SetDataMask(int totalitems, Headlist *Heads, int PartID, int PartNum) {
    int i, n = Heads->size;

    tran.setmaxsize(n);

    mask = new char[n];
    memset(mask,0,sizeof(char)*n);

    //create index list, index = 0
    index = new int[totalitems];

    //set index of frequent items
    memset(index, 0, sizeof(int) * totalitems);
    for (i = 0; i < n; i++) index[Heads->items[i].id] = (i % PartNum == PartID) ? -(i+1) : (i+1);
}


Transaction* ProcessTransactions::GetTransaction()
{
    //if there is an transaction in the bag
    if (transactions.empty() && (Flag == true))
    {
        pair<set<Transaction>::iterator,bool> ret;
        set<Transaction>::iterator it;
        Transaction *p;
        Transaction t;
        int count = 0;
        t.count = 1;

        while (count < TRANSACTION_BUFFER_SIZE)
        {
            if (p = GetTransaction())
            {
                t.size = p->size;
                t.items = new int[p->size];
                memcpy(t.items,p->items,sizeof(int)*p->size);
                ret = trans.insert(t);

                if (ret.second==false)
                {	//item exist
                    it = ret.first;
                    it->count = it->count + 1;
                    //			delete[] t.items;
                }
                count++;
            }
            else
            {
                Flag = false;
                break;
            }
        }

        if (count)
        {
            t.items = 0; //must have this line
            ptran = trans.begin();
        }
    }

    if (trans.size())
    {
        tran.size = ptran->size;
        tran.count = ptran->count;
        memcpy(tran.items,ptran->items,sizeof(int)*tran.size);
        set<Transaction>::iterator it = ptran;
        ptran++;
        trans.erase(it);
        return &tran;
    }
    else
    {
        Flag = true;
        return 0;
    }
}
