//
// Created by evan on 10/25/18.
//

#include "../../headers/cluster/ShaMC.hpp"

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
    FPM_modified fpm;
    SharedSubspace* subspace;

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
#pragma omp parallel for schedule(dynamic) shared(centroid, parameters, minPoints, currentSize, clusterCount, failedAttempts, i, sharedInfo, subspace) private(me, fpm)
            for (me = 0; me < omp_get_num_threads(); me++)
            {
                ProcessTransactions transactions(X.getPartitionSize(me));
                transactions.buildTransactionsPar(centroid.first, X, me);
                subspace = fpm.Mine_Patterns_Parallel(transactions, 1, 2, &sharedInfo, parameters);
                for (auto &item : subspace->getSubspaces()){
                    std::cout << item->itemset << ", " << item->count << std::endl;
                }
                std::cout << std::endl;
            }


        }

        end = omp_get_wtime();

        log_info("Iteration " + std::to_string(i) + " took " + std::to_string(end-start) + "s");
    }
}
