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

    explicit SharedSubspace(SharedSettings &parameters): _parameters{parameters} {};

    void buildSubspace(std::stringstream &dimensionSet);

    uint64_t clusterPar(RowIndex centroidID, SharedDataset &X, PartitionID me, int clusterNum);

    void printCentroid();
};


#endif //SHAMC_PROCESSSUBSPACE_HPP
