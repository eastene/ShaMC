//
// Created by evan on 10/25/18.
//

#include <cstring>
#include "../../headers/cluster/ShaMC.hpp"

void ShaMC::fit(SharedDataset &X) {
    bool isOk = true;
    bool failed = true;
    RowIndex clusterCount = 0;
    RowIndex currentSize = X.shape().first;
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    PartitionID me;
    SharedSubspace subspace(parameters);
    double start, end;
    int num_clusts = 0;
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

        DimensionSet bestSubspace;

        int inner_threads = parameters.nThreads > mediods.size() ? parameters.nThreads / mediods.size() : 1;
        // resize per-mediod data structures to match number of mediods
        mediod_transactions.resize(mediods.size());
        mediod_frequent_items.resize(mediods.size());
        mediod_tot_points.resize(mediods.size());
        sharedInfos.resize(mediods.size());
        X.repartition(inner_threads);

#pragma omp parallel for schedule(static) shared(mediod_transactions, mediod_frequent_items, sharedInfos) private(me)
        for (int k = 0; k < mediods.size() * inner_threads; k++){
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
#pragma omp critical
            *mediod_transactions[m] << transactions.getTransactions()->str();
        }

        DimensionSet tempset;
        for (int k = 0; k < mediods.size(); k++) {
#pragma omp parallel private(me)
            {
                auto mediod = mediods.begin();
                std::advance(mediod, k);
                ParFPM pfpm;
                auto myInput = new std::stringstream;
                myInput->str(mediod_transactions[k]->str());
                pfpm.Mine_Patterns(myInput, mediod_frequent_items[k], support, 128, 1, sharedInfos[k]);
                delete myInput;
#pragma omp barrier

                if (omp_get_thread_num() == 0) {
                    std::cout << mediod_frequent_items[k]->str() << std::endl;
                    tempset = subspace.buildSubspace(mediod_frequent_items[k], mediod->first);

                    if (subspace.compareSubspaces(tempset, bestSubspace) == 1)
                        bestSubspace = tempset;

                    delete mediod_transactions[k];
                    delete mediod_frequent_items[k];
                    delete sharedInfos[k];
                }
            }
        }

        if (bestSubspace.mu == 0){
            failedAttempts++;
            continue; // retry
        } else{
            failedAttempts = 0;
        }

        X.repartition(omp_get_num_threads());
#pragma omp parallel private(me)
        {
            uint64_t points;
            me = omp_get_num_threads();
            points = subspace.clusterPar(X, me, num_clusts, bestSubspace);

#pragma omp atomic
            bestSubspace.numPoints += points;
        }

        std::cout << "New cluster " << num_clusts++ << " found" << std::endl;
        std::cout << "  Points: " << bestSubspace.numPoints << std::endl;
        std::cout << "  Dimensions: " << bestSubspace.itemset.size() << std::endl;
    }

    if (failedAttempts >= parameters.maxAttempts) {
        std::cout << "No new clusters found in " << failedAttempts << " attempts." << std::endl;
    }

    end = omp_get_wtime();
    X.printSummaryStats();

    std::cout << "Total Elapsed time: " << (end - start) << "s";
}
