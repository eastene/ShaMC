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
    std::uniform_int_distribution<unsigned long> dis(0, X.shape().first - 1);
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
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    int iteration;
    uint32_t i;
    PartitionID me;
    FPM fpm;

    for (i = 0; i < parameters.maxiter; i++) {

        // termination conditions
        if (currentSize <= minPoints || failedAttempts >= parameters.maxAttempts) {
            break;
        }

        clusterCount = currentSize <= parameters.mediods ? currentSize : parameters.mediods;
        mediods = pickMediodsRandom(X, clusterCount);

        for (const auto &centroid : mediods) {
#pragma omp parallel for schedule(dynamic) shared(centroid, parameters, minPoints, currentSize, clusterCount, failedAttempts, i) private(me)
            for (me = 0; me < omp_get_num_threads(); me++) {
                Transactions transactions(X.getPartitionSize(me));
                buildTransactionsPar(centroid.first, X, me, &transactions);
                //fpm.Mine_Patterns_Parallel();
            }
        }
    }
}
