//
// Created by evan on 10/25/18.
//

#include <cstring>
#include "../../headers/cluster/ShaMC.hpp"

void ShaMC::fit(SharedDataset &X) {
    bool isOk = true;
    RowIndex clusterCount = 0;
    RowIndex currentSize = X.shape().first;
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    PartitionID me;
    SharedSubspace subspace(parameters);
    double start, end;
    int support = X.getSupport();

    // per-mediod datastructures
    std::vector<std::stringstream *> mediod_frequent_items;
    std::vector<std::stringstream *> mediod_transactions;
    std::vector<uint64_t> mediod_tot_points;
    std::vector<Info *> sharedInfos;

    start = omp_get_wtime();
    for (int i = 0; i < parameters.maxiter; i++) {

        // termination conditions
        if (currentSize <= minPoints || failedAttempts >= parameters.maxAttempts) {
            break;
        }

        clusterCount = currentSize <= parameters.mediods ? currentSize : parameters.mediods;
        mediods = X.pickMediodsRandom(clusterCount);

        int inner_threads = parameters.nThreads > mediods.size() ? parameters.nThreads / mediods.size() : 1;
        // resize per-mediod data structures to match number of mediods
        mediod_transactions.resize(mediods.size());
        mediod_frequent_items.resize(mediods.size());
        mediod_tot_points.resize(mediods.size());
        sharedInfos.resize(mediods.size());
        X.repartition(inner_threads);

#pragma omp parallel for private(me)
        for (int k = 0; k < mediods.size(); k++){
            int m = k / inner_threads;
            SharedTransactions transactions(1);
            auto mediod = mediods.begin();
            std::advance(mediod, m);
            me = omp_get_thread_num() % inner_threads;
            mediod_tot_points[m] = 0;

            if (me == 0) {
                mediod_transactions[m] = new std::stringstream;
                mediod_frequent_items[m] = new std::stringstream;
                sharedInfos[m] = new Info; // allocate here, not needed until later
            }

            // each transaction object only has half of the transactions for a mediod
            transactions.buildTransactionsPar(mediod->first, X, me);

            // once each thread has finished counting transactions, add them all to the respective
            // total for that mediod
//#pragma omp barrier
#pragma omp critical
            *mediod_transactions[m] << transactions.getTransactions()->str();
        }

        X.repartition(omp_get_num_threads());
        for (int k = 0; k < mediods.size(); k++) {
#pragma omp parallel private(me)
            {
                auto mediod = mediods.begin();
                std::advance(mediod, k);
                ParFPM pfpm;
                uint64_t points = 0;
                me = omp_get_thread_num();
                auto myInput = new std::stringstream;
                myInput->str(mediod_transactions[k]->str());
                pfpm.Mine_Patterns(myInput, mediod_frequent_items[k], support, 128, 1, sharedInfos[k]);
                delete myInput;
#pragma omp barrier
                // TODO implement check if frequent items is empty
                myInput = new std::stringstream;
                myInput->str(mediod_frequent_items[k]->str());

                if (omp_get_thread_num() == 0)
                    subspace.buildSubspace(myInput, mediod->first);
#pragma omp barrier
                points = subspace.clusterPar(X, me, k);

#pragma omp atomic
                mediod_tot_points[k] += points;

                if (omp_get_thread_num() == 0) {
                    delete mediod_transactions[k];
                    delete mediod_frequent_items[k];
                    delete sharedInfos[k];
                }
            }
        }
    }

    end = omp_get_wtime();
    X.printSummaryStats();

    std::cout << "Total Elapsed time: " << (end - start) << "s";
}
