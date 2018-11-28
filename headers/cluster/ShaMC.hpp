//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_SHAMC_HPP
#define SHAMC_SHAMC_HPP

#include <omp.h>
#include <vector>
#include <random>
#include <unordered_map>
#include "../utils/SharedDataset.hpp"
#include "../utils/SharedSettings.hpp"
#include "../../headers/utils/logger.h"
#include "../../ShaFEM-MEM/FPM.h"
#include "../utils/SharedTransactions.hpp"
#include "../../headers/utils/SharedSubspace.hpp"

class ShaMC {
private:
    MultiRowMap mediods;
    SharedSettings parameters;

public:
    explicit ShaMC(SharedSettings &parameters) { this->parameters = parameters; }

    // find mediods using subspace clustering, overwrites any existing mediods
    void fit(SharedDataset &X, uint16_t nThreads = 1);

    // predict labels using mediods found after fitting
    SharedDataset predict(SharedDataset &X);

    // perform fitting and prediction in single step (useful for training set)
    SharedDataset fit_predict(SharedDataset &X);
};


#endif //SHAMC_SHAMC_HPP
