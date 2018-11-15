//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_SHAMC_HPP
#define SHAMC_SHAMC_HPP

#include <omp.h>
#include <vector>
#include <unordered_map>
#include "../utils/SharedDataset.hpp"
#include "../utils/SharedSettings.hpp"

//#include "../ShaFEM/FPM.h"

typedef std::vector<uint16_t> DimensionSet;
typedef std::vector<DimensionSet> Transactions;

class ShaMC {
private:
    MultiRowMap mediods;
    SharedSettings parameters;

    MultiRowMap pickMediodsRandom(SharedDataset &X, RowIndex n);

    void buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me, Transactions *transactions);

public:
    ShaMC(SharedSettings &parameters) { this->parameters = parameters; }

    // find mediods using subspace clustering, overwrites any existing mediods
    void fit(SharedDataset &X);

    // predict labels using mediods found after fitting
    SharedDataset predict(SharedDataset &X);

    // perform fitting and prediction in single step (useful for training set)
    SharedDataset fit_predict(SharedDataset &X);
};


#endif //SHAMC_SHAMC_HPP
