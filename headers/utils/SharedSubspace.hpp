//
// Created by evan on 11/19/18.
//

#ifndef SHAMC_PROCESSSUBSPACE_HPP
#define SHAMC_PROCESSSUBSPACE_HPP


#include <cstdint>
#include <vector>
#include <set>
#include "SharedTransactions.hpp"

struct DimensionSet {
    RowIndex mediodID;
    std::vector<int> itemset;
    int count = 0;
    double mu = 0;
    uint64_t numPoints = 0;
};

class SharedSubspace {
private:
    SharedSettings _parameters;

public:

    explicit SharedSubspace(SharedSettings &parameters):_parameters{parameters}{};

    DimensionSet buildSubspace(std::stringstream *dimensionSet, RowIndex mediodID);

    uint64_t clusterPar(SharedDataset &X, PartitionID me, int clusterNum, DimensionSet &subspace);

    bool compareSubspaces(DimensionSet &subspace1, DimensionSet &subspace2);

    void printCentroid(DimensionSet centroid);
};


#endif //SHAMC_PROCESSSUBSPACE_HPP
