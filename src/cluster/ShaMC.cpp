//
// Created by evan on 10/25/18.
//

#include <omp.h>
#include <random>
#include "../../headers/cluster/ShaMC.hpp"
#include "../../headers/utils/logger.h"
#include "../../headers/ShaFEM/FPM_modified.hpp"
#include "../../headers/utils/ProcessTransactions.hpp"
#include "../../headers/ShaFEM/FPM.h"

void ShaMC::fit(SharedDataset &X) {
    RowIndex clusterCount = 0;
    RowIndex currentSize = X.shape().first;
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    int iteration;
    uint32_t i;
    PartitionID me;
    double start, end;
    //FPM_modified fpm;
    FPM fpm;

    for (i = 0; i < parameters.maxiter; i++) {
        start = omp_get_wtime();
        // termination conditions
        if (currentSize <= minPoints || failedAttempts >= parameters.maxAttempts) {
            break;
        }

        clusterCount = currentSize <= parameters.mediods ? currentSize : parameters.mediods;
        mediods = X.pickMediodsRandom(clusterCount);

        for (const auto &centroid : mediods) {
            Info sharedInfo;
#pragma omp parallel for schedule(dynamic) shared(centroid, parameters, minPoints, currentSize, clusterCount, failedAttempts, i) private(me)
            for (me = 0; me < omp_get_num_threads(); me++) {
                ProcessTransactions transactions(X.getPartitionSize(me));
                transactions.buildTransactionsPar(centroid.first, X, me);
                // TODO: try using FPGrowth and writing to disk, then using in-memory data structures
                fpm.DFEM();
                //fpm.Mine_Patterns_Parallel(transactions, 5, 4, 1, &sharedInfo);
            }
        }

        end = omp_get_wtime();

        log_info("Iteration " + std::to_string(i) + " took " + std::to_string(end-start) + "s");
    }
}
