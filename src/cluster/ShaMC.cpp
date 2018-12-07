//
// Created by evan on 10/25/18.
//

#include <cstring>
#include "../../headers/cluster/ShaMC.hpp"

void ShaMC::fit(SharedDataset &X) {
    RowIndex clusterCount = 0;
    RowIndex currentSize = X.shape().first;
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    PartitionID me;
    SharedSubspace subspace(parameters);
    double start, end;
    int num_clusts_found = 0;
    int support = X.getSupport();

    // per-mediod datastructures
    std::vector<std::stringstream *> mediod_frequent_items;
    std::vector<std::stringstream *> mediod_transactions;
    std::vector<Info *> sharedInfos;

    start = omp_get_wtime();
    int i;
    for (i = 0; i < parameters.maxiter; i++) {

        // termination conditions
        if (currentSize <= minPoints || failedAttempts >= parameters.maxAttempts) {
            std::cout << "Not enough points remaining to form cluster. Stopping." << std::endl;
            break;
        }

        clusterCount = currentSize <= parameters.mediods ? currentSize : parameters.mediods;
        mediods = X.pickMediodsRandom(clusterCount);



        //int inner_threads = parameters.nThreads > mediods.size() ? parameters.nThreads / mediods.size() : 1;
        // resize per-mediod data structures to match number of mediods
        mediod_transactions.resize(mediods.size());
        mediod_frequent_items.resize(mediods.size());
        sharedInfos.resize(mediods.size());
        //X.repartition(inner_threads);

//#pragma omp parallel for schedule(static) shared(mediod_transactions, mediod_frequent_items, sharedInfos) private(me)
        DimensionSet bestSubspace;
        DimensionSet tempset;
        for (int k = 0; k < mediods.size(); k++) {
#pragma omp parallel private(me)
            {
                SharedTransactions transactions(1);
                auto mediod = mediods.begin();
                std::advance(mediod, k);
                me = omp_get_thread_num();

                if (me == 0) {
                    mediod_transactions[k] = new std::stringstream;
                    mediod_frequent_items[k] = new std::stringstream;
                    sharedInfos[k] = new Info; // allocate here, not needed until later
                }

                // each transaction object only has half of the transactions for a mediod
                transactions.buildTransactionsPar(mediod->first, X, me);

                // once each thread has finished counting transactions, add them all to the respective
                // total for that mediod
#pragma omp barrier
#pragma omp critical
                *mediod_transactions[k] << transactions.getTransactions()->str();


                ParFPM pfpm;
                auto myInput = new std::stringstream;
#pragma omp barrier
                myInput->str(mediod_transactions[k]->str());
                pfpm.Mine_Patterns(myInput, mediod_frequent_items[k], support, 128, 1, sharedInfos[k]);
                delete myInput;
#pragma omp barrier

                if (omp_get_thread_num() == 0) {
                    tempset = subspace.buildSubspace(mediod_frequent_items[k], mediod->first);

                    // update best subspace
                    if (subspace.compareSubspaces(tempset, bestSubspace))
                        bestSubspace = tempset;

                    // delete stringstream pointers (clearing them doesn't seem to work)
                    delete mediod_transactions[k];
                    delete mediod_frequent_items[k];
                    delete sharedInfos[k];
                }
            }
        }

        if (bestSubspace.mu == 0) {
            failedAttempts++;
            continue; // retry
        } else {
            failedAttempts = 0;
        }

#pragma omp parallel private(me)
        {
            uint64_t points;
            me = omp_get_thread_num();
            points = subspace.clusterPar(X, me, num_clusts_found, bestSubspace);

#pragma omp atomic
            bestSubspace.numPoints += points;
        }

        std::cout << "New cluster " << num_clusts_found++ << " found" << std::endl;
        std::cout<< "  Support: " << bestSubspace.count << std::endl;
        std::cout << "  Points: " << bestSubspace.numPoints << std::endl;
        std::cout << "  Dimensions: " << bestSubspace.itemset.size() << std::endl;

        bestSubspace.mu = 0.0; bestSubspace.numPoints = 0; bestSubspace.itemset.clear(); bestSubspace.count = 0;
        currentSize = X.getNumUnclustered();
    }

    if (failedAttempts >= parameters.maxAttempts) {
        std::cout << "No new clusters found in " << failedAttempts << " attempts." << std::endl;
    }

    if (i >= parameters.maxiter) {
        std::cout << "Stopped after " << i << " iterations." << std::endl;
    }

    end = omp_get_wtime();
    X.printSummaryStats();

    std::cout << "Total Elapsed time: " << (end - start) << "s";
}
