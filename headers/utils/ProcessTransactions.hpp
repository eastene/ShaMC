//
// Created by evan on 11/16/18.
//

#ifndef SHAMC_SHAREDTRANSACTIONS_HPP
#define SHAMC_SHAREDTRANSACTIONS_HPP

#include <vector>
#include "../ShaFEM/DataObject.h"
#include "SharedDataset.hpp"

namespace ProcessTransactions {
    typedef uint16_t Dimension;
    typedef std::vector<Dimension> Transaction;

    class ProcessTransactions {
    private:
        std::vector<Transaction> transactions;
        Countlist counts;
        uint32_t numTransactions;

    public:
        ProcessTransactions(){numTransactions = 0;}
        explicit ProcessTransactions(uint64_t size) {transactions.resize(size); numTransactions=0;}

        void buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me);

        Countlist getCounts() {return counts;}

        uint32_t getNumTransactions(){return numTransactions;}
    };
}




#endif //SHAMC_SHAREDTRANSACTIONS_HPP
