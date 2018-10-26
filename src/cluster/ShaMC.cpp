//
// Created by evan on 10/25/18.
//

#include <random>
#include "../../headers/cluster/ShaMC.hpp"
#include "../../headers/utils/logger.h"

MultiRow ShaMC::pickMediodsRandom(SharedDataset &X, RowIndex n) {
    MultiRow _mediods(n);

    // random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned long> dis(1, X.shape().first);
    RowIndex i;

    // randomly choose n mediods
    while (_mediods.size() < n) {
        i = dis(gen);
        if (mediods.find(i) == mediods.end()) {
            _mediods[i] = X.getRowAsynch(i);
        }
    }

    return _mediods;
}


void ShaMC::buildTransactionsPar(std::string centriodID, ) {

}


void ShaMC::fit(SharedDataset &X) {
    RowIndex clusterCount = 0;
    RowIndex currentSize = X.shape().first;
    Partition partition{0, 0};
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    int iteration;
    bool isOk = true;
    uint32_t i;

#pragma omp parallel shared(clusterCount, failedAttempts, i) private(partition)
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
                    log_info("Iteration " + std::to_string(iteration) + " out of " + std::to_string(mediods.size()));
                    buildTransactionsPar(centroid.second.id);
                }
            }

        }
    }
}