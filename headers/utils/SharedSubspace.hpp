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
    int count;
    double mu;
    uint64_t numPoints;
};

class SharedSubspace {
private:
    SharedSettings _parameters;
    DimensionSet _centroid;

public:

    explicit SharedSubspace(SharedSettings &parameters):_parameters{parameters}{};

    void buildSubspace(std::stringstream *dimensionSet, RowIndex mediodID);

    uint64_t clusterPar(SharedDataset &X, PartitionID me, int clusterNum);

    void printCentroid(DimensionSet centroid);
};


#endif //SHAMC_PROCESSSUBSPACE_HPP
