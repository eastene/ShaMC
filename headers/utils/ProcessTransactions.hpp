//
// Created by evan on 11/16/18.
//

#ifndef SHAMC_SHAREDTRANSACTIONS_HPP
#define SHAMC_SHAREDTRANSACTIONS_HPP

#include <vector>
#include "../ShaFEM/DataObject.h"
#include "SharedDataset.hpp"

typedef uint16_t Dimension;
typedef std::vector<Dimension> DimensionSet;

class ProcessTransactions {
private:
    std::vector<DimensionSet> transactions;
    Countlist counts;
    uint32_t numTransactions;

public:
    int *index;	//list of mapping index of original item
                //index = 0 : infrequent items
                //index = x : item with index (x-1) is frequent
                //index = -x: item with index (x-1) is frequent and the transaction must contain this item
    char *mask;
    Transaction tran;

    ProcessTransactions() { numTransactions = 0; }

    explicit ProcessTransactions(uint64_t size) {
        transactions.resize(size);
        numTransactions = 0;
    }

    void buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me);

    void SetDataMask(int totalitems, Headlist *Heads, int PartID = 0, int PartNum = 1);

    Transaction* GetTransaction();

    Countlist getCounts() { return counts; }

    uint32_t getNumTransactions() { return numTransactions; }
};

#endif //SHAMC_SHAREDTRANSACTIONS_HPP
