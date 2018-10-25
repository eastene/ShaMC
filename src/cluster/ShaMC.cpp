//
// Created by evan on 10/25/18.
//

#include <random>
#include <unordered_set>
#include "../../headers/cluster/ShaMC.hpp"

MultiRow ShaMC::pickCentroidsRandom(SharedDataset &X, unsigned int n) {
    MultiRow _centroids(n);
    std::unordered_set<int> mediods;

    // random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<RowIndex > dis(1, X.shape().first);
    RowIndex i;

    while (_centroids.size() < n) {
        i = dis(gen);
        if (mediods.find(i) == mediods.end()) {
            mediods.emplace(i);
            _centroids[i] = X.getRowAsynch(i);
        }
    }

    return _centroids;
}

void ShaMC::fit(SharedDataset &X) {
    int clusterCount = 0;
    int currentSize = X.shape.first;
    int minPoints = (int) ceil(parameters.alpha * X.shape().first);
    int failedAttempts = 0;

    while (true) {
        // termination conditions
        if (currentSize <= minPoints || failedAttempts >= parameters.maxAttempts) {
            break;
        }

        clusterCount = currentSize <= parameters.mediods ? currentSize : parameters.mediods;
        centroids = pickCentroidsRandom(clusterCount);
    }

#pragma omp parallel
    {


    }
}