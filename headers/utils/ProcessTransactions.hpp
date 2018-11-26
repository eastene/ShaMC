//
// Created by evan on 11/16/18.
//

#ifndef SHAMC_SHAREDTRANSACTIONS_HPP
#define SHAMC_SHAREDTRANSACTIONS_HPP

#include <vector>
#include "../ShaFEM/DataObject.h"
#include "SharedDataset.hpp"

typedef int Dimension;
class Dimensions {
private:
    std::vector<Dimension> _items;
    int _count;
public:
    Dimensions(){_count=0;}
    void add(Dimension d){_items.emplace_back(d); }
    void setMaxSize(int n){_items.resize(n);}
    Dimension &operator[](int index) { return _items[index]; }

    const std::vector<Dimension> getItems(){return _items;}
    int getSize(){return _items.size();}
    int getCount(){return _count;}
};

class ProcessTransactions {
private:
    std::vector<Dimensions> transactions;
    uint64_t currTransaction;
    Countlist counts;
    uint32_t numTransactions;

public:
    int *index;	//list of mapping index of original itemset
                //index = 0 : infrequent items
                //index = x : itemset with index (x-1) is frequent
                //index = -x: itemset with index (x-1) is frequent and the transaction must contain this itemset
    char *mask;
    Transaction *t;

    ProcessTransactions() { numTransactions = 0; currTransaction = 0;}

    explicit ProcessTransactions(uint64_t size) {
        transactions.resize(size);
        numTransactions = 0; currTransaction = 0;
    }

    ~ProcessTransactions(){delete t;}

    void buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me);

    void SetDataMask(int totalitems, Headlist *Heads, int PartID = 0, int PartNum = 1);

    Transaction* getTransaction();

    bool moreTransactions(){return currTransaction < transactions.size();}

    Countlist getCounts() { return counts; }

    uint32_t getNumTransactions() { return numTransactions; }
};

#endif //SHAMC_SHAREDTRANSACTIONS_HPP
