//
// Created by evan on 10/25/18.
//

#include <cstring>
#include "../../headers/cluster/ShaMC.hpp"

void ShaMC::fit(SharedDataset &X, uint16_t nThreads) {
    bool isOk = true;
    RowIndex clusterCount = 0;
    RowIndex currentSize = X.shape().first;
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;
    PartitionID me;
    double start, end;
    Info *sharedInfo;
    std::vector<std::stringstream *> mediod_frequent_items;
    std::vector<std::stringstream *> mediod_transactions;
    std::vector<uint64_t> mediod_tot_points;

    for (int i = 0; i < parameters.maxiter; i++) {

        start = omp_get_wtime();

        // termination conditions
        if (currentSize <= minPoints || failedAttempts >= parameters.maxAttempts) {
            break;
        }

        clusterCount = currentSize <= parameters.mediods ? currentSize : parameters.mediods;
        mediods = X.pickMediodsRandom(clusterCount);

        int inner_threads = parameters.nThreads > mediods.size() ? parameters.nThreads / mediods.size() : 1;
        mediod_transactions.resize(mediods.size());
        mediod_frequent_items.resize(mediods.size());
        mediod_tot_points.resize(mediods.size());
        X.repartition(inner_threads);

        std::vector<omp_lock_t> lock_m(mediods.size());
        for (auto lock : lock_m)
            omp_init_lock(&lock);

#pragma parallel omp for schedule(static) private(me)
        for (int k = 0; k < mediods.size() * inner_threads; k++) {
            int m = k / inner_threads;
            SharedTransactions transactions(1);
            auto mediod = mediods.begin();
            std::advance(mediod, m);
            me = omp_get_thread_num() % inner_threads;
            mediod_tot_points[m] = 0;

            omp_set_lock(&lock_m[m]);
            if (me == 0) {
                mediod_transactions[m] = new std::stringstream;
                mediod_frequent_items[m] = new std::stringstream;

            }
            omp_unset_lock(&lock_m[m]);

            // each transaction object only has half of the transactions for a mediod
            transactions.buildTransactionsPar(mediod->first, X, me);

            omp_set_lock(&lock_m[m]);
            *mediod_transactions[m] << transactions.getTransactions()->str();
            omp_unset_lock(&lock_m[m]);
        }

        SharedSubspace subspace(parameters);
        for (int m = 0; m < mediods.size(); m++) {
#pragma omp parallel private(me)
            {
                ParFPM pfpm;
                uint64_t points = 0;
                auto mediod = mediods.begin();
                std::advance(mediod, m);
                auto myInput = new std::stringstream;
                me = omp_get_thread_num();

                myInput->str(mediod_transactions[m]->str());
                if (omp_get_thread_num() == 0)
                    sharedInfo = new Info;
#pragma omp barrier
                pfpm.Mine_Patterns(myInput, mediod_frequent_items[m], 1, 128, 1, sharedInfo);
                delete myInput;

                //DimensionSet centroid = subspace.buildSubspace(*mediod_frequent_items[m], mediod->first);

                //points = subspace.clusterPar(centroid, X, me, m);

//#pragma omp atomic
                //mediod_tot_points[m] += points;

                if (me == 0) {
                    //std::cout << mediod_tot_points[m] << std::endl;
                    std::cout << mediod_frequent_items[m]->str() << std::endl;
                    omp_set_lock(&lock_m[m]);
                    delete mediod_transactions[m];
                    delete mediod_frequent_items[m];
                    omp_unset_lock(&lock_m[m]);
                }

                if (omp_get_thread_num() == 0)
                    delete sharedInfo;
            }
        }

        end = omp_get_wtime();
        std::cout << "Iteration " << i << " took " << (end - start) << "s" << std::endl;
    }
}
