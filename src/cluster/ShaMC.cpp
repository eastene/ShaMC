//
// Created by evan on 10/25/18.
//

#include "../../headers/cluster/ShaMC.hpp"

void ShaMC::fit(SharedDataset &X, uint16_t nThreads) {
    bool isOk = true;
    RowIndex clusterCount = 0;
    RowIndex currentSize = X.shape().first;
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    PartitionID me;
    double start, end;
    //FPM_modified fpm;
    ParFPM pfpm;
    Info *sharedInfo = new Info;
    std::stringstream *frequent_items;
    SharedTransactions transactions(nThreads);
    uint64_t totPoints;

    for (int i = 0; i < parameters.maxiter; i++) {

        start = omp_get_wtime();

        // termination conditions
        if (currentSize <= minPoints || failedAttempts >= parameters.maxAttempts) {
            break;
        }

        clusterCount = currentSize <= parameters.mediods ? currentSize : parameters.mediods;
        mediods = X.pickMediodsRandom(clusterCount);

        int j = 0;
        for (const auto &centroid : mediods) {
#pragma omp parallel shared(isOk, totPoints, sharedInfo, frequent_items, transactions, i, j, centroid, X) private(pfpm, me)
            {
                SharedSubspace subspace(parameters);
                uint64_t points = 0;
                totPoints = 0;

#pragma omp single
                {
                    frequent_items = new std::stringstream;
                }

                me = omp_get_thread_num();
                transactions.buildTransactionsPar(centroid.first, X, me);
#pragma omp barrier

#pragma omp single
                {
                    transactions.reduce();
                    if (transactions.getNumTransactions() == 0)
                        isOk = false;
                }

                if (isOk)
                    pfpm.Mine_Patterns(transactions.getTransactions(), frequent_items, 1, 2, 1, sharedInfo,
                                       transactions.getNumTransactions());
#pragma omp single
                {
                    std::cout << frequent_items->str() << std::endl;
                    if (frequent_items->str().empty())  // no frequent items found
                        isOk = false;
                    else
                        subspace.buildSubspace(*frequent_items);
                }

                if (isOk) {
                    points = subspace.clusterPar(centroid.first, X, me, j);
#pragma omp critical
                    totPoints += points;
                }

#pragma omp single
                {
                    j++;

                    delete frequent_items;
                }

                isOk = true;
            }
        }

        end = omp_get_wtime();
        log_info("Iteration " + std::to_string(i) + " took " + std::to_string(end - start) + "s");
    }

    delete sharedInfo;
}
