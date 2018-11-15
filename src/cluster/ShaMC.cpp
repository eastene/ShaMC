//
// Created by evan on 10/25/18.
//

#include <random>
#include "../../headers/cluster/ShaMC.hpp"
#include "../../headers/utils/logger.h"

MultiRowMap ShaMC::pickMediodsRandom(SharedDataset &X, RowIndex n) {
    MultiRowMap _mediods(n);

    // random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned long> dis(1, X.shape().first);
    RowIndex i;
    RowIndex chosen = 0;

    // randomly choose n mediods
    while (chosen < n) {
        i = dis(gen);
        if (mediods.find(i) == mediods.end()) {
            _mediods[i] = X.getRow(i);
            chosen++;
        }
    }

    return _mediods;
}


void ShaMC::buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me, Transactions *transactions) {
    Row *centroid = X.getRow(centroidID);
    Row *point;

    for (uint64_t i = 0; i < X.getPartitionSize(me); i++) {
        if (i == centroid->idx)
            continue;

        point = X.getRowFromPartition(i, me);
        for (uint16_t j = 0; j < point->cells.size(); j++) {
            if (fabs(point->cells[j] - centroid->cells[j]) <= parameters.width)
                (*transactions)[i].push_back(j);
        }
    }
}


void ShaMC::fit(SharedDataset &X) {
    RowIndex clusterCount = 0;
    RowIndex currentSize = X.shape().first;
    PartitionID me = 0;
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    int iteration;
    bool isOk = true;
    uint32_t i;

#pragma omp parallel shared(clusterCount, failedAttempts, i) private(me)
    {
        for (i = 0; i < parameters.maxiter; i++) {
            if (isOk) {
                // termination conditions
#pragma omp single
                if (currentSize <= minPoints || failedAttempts >= parameters.maxAttempts) {
                    isOk = false;
                }

                clusterCount = currentSize <= parameters.mediods ? currentSize : parameters.mediods;
                mediods = pickMediodsRandom(X, clusterCount);

                iteration = 1;
                for (const auto &centroid : mediods) {
                    //log_info("Iteration " + std::to_string(iteration) + " out of " + std::to_string(mediods.size()));
                    std::cout << "Mediod: " << centroid.first << std::endl;
                    Transactions transactions(X.getPartitionSize(me));
                    buildTransactionsPar(centroid.first, X, me, &transactions);
                }
            }

        }
    }
}