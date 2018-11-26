//
// Created by evan on 11/16/18.
//

#include <cmath>
#include <cstring>

#include "../../headers/utils/ProcessTransactions.hpp"

void ProcessTransactions::buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me) {
    Row *centroid = X.getRow(centroidID);
    Row *point;

    for (uint64_t i = 0; i < X.getPartitionSize(me); i++) {
        if (i == centroid->idx)
            continue;

        point = X.getRowFromPartition(i, me);
        for (Dimension j = 1; j <= point->cells.size(); j++) {
            if (fabs(point->cells[j] - centroid->cells[j]) <= X.getSettings().width) {
                this->transactions[i].add(j);
                this->counts.count(j);
            }
        }
        numTransactions++;
    }
}

void ProcessTransactions::SetDataMask(int totalitems, Headlist *Heads, int PartID, int PartNum) {
    int i, n = Heads->size;

    t = new Transaction;
    t->setmaxsize(n);

    mask = new char[n];
    memset(mask, 0, sizeof(char) * n);

    //create index list, index = 0
    index = new int[totalitems];

    //set index of frequent items
    memset(index, 0, sizeof(int) * totalitems);
    for (i = 0; i < n; i++) index[Heads->items[i].id] = (i % PartNum == PartID) ? -(i + 1) : (i + 1);
}

Transaction *ProcessTransactions::getTransaction() {
    t->size = 0;
    if (currTransaction < transactions.size()) {
        for (auto d : transactions[currTransaction++].getItems()) {
            t->items[t->size++] = d;
        }
        t->sort();
        return t;
    }
    else
        throw OutOfRangeException();
}
