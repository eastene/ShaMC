//
// Created by evan on 11/16/18.
//

#ifndef SHAMC_SHAREDTRANSACTIONS_HPP
#define SHAMC_SHAREDTRANSACTIONS_HPP

#include <vector>
#include "SharedDataset.hpp"
#include "../../ShaFEM-MEM/DataObject.h"

typedef int Dimension;

class SharedTransactions {
private:

    std::stringstream*_transactions;
    std::vector<std::shared_ptr<std::stringstream>> _parStreams;

    uint32_t _numTransactions;
    uint64_t _numItems;
    uint16_t _numThreads;
    bool _reduced;

    //Countlist _counts;

public:

    SharedTransactions() {
        _numThreads = 1;
        _parStreams.resize(_numThreads);
        _parStreams[0] = std::make_shared<std::stringstream>();
        _transactions = new std::stringstream("");
        _numTransactions = 0;
        _numItems = 0;
        _reduced = false;
    }

    explicit SharedTransactions(uint16_t numThreads) {
        _numThreads = numThreads;
        _parStreams.resize(numThreads);
        for (int i = 0; i < numThreads; i++)
            _parStreams[i] = std::make_shared<std::stringstream>();
        _transactions = new std::stringstream("");
        _numTransactions = 0;
        _numItems = 0;
        _reduced = false;
    }

    void buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me);

    uint32_t getNumTransactions() { return _numTransactions; }

    uint64_t getNumItems() { return _numItems; }

    void reduce();

    std::stringstream* getTransactions(){if (_reduced) ; else reduce(); return _transactions;}

   // Countlist& getCounts() { return _counts; }
};

#endif //SHAMC_SHAREDTRANSACTIONS_HPP
