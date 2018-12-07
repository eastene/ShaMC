//
// Created by evan on 11/19/18.
//

#include <iostream>
#include "../../headers/utils/SharedSubspace.hpp"

DimensionSet SharedSubspace::buildSubspace(std::stringstream *dimensionSet, RowIndex mediodID) {
    std::string line;
    std::string token;
    int support = 0;
    std::vector<int> tmpDimSet;
    double mu_best = 0.0;
    DimensionSet subspace;
    subspace.mediodID = mediodID;

    subspace.count = 0;
    subspace.itemset = tmpDimSet;
    subspace.mu = 0.0;
    subspace.numPoints = 0;

    if (!dimensionSet || dimensionSet->str().empty())
        return subspace;

    while (!std::getline(*dimensionSet, line).eof()) {
        std::stringstream ss(line);
        while (std::getline(ss, token, ' ')) {
            if (token[0] == '(') {
                support = std::stoi(token.substr(1, token.size() - 2)); // support in parens
            } else {
                tmpDimSet.push_back(std::stoi(token));
            }
        }
        double mu = support * pow((1 / _parameters.beta), tmpDimSet.size());

        if (mu > mu_best) {
            subspace.count = support;
            subspace.itemset = tmpDimSet;
            subspace.mu = mu;
            mu_best = mu;
        }

        tmpDimSet.clear();
    }

    return subspace;
}

uint64_t SharedSubspace::clusterPar(SharedDataset &X, PartitionID me, int clusterNum, DimensionSet &subspace) {
    Row *mediod = X.getRow(subspace.mediodID);
    Row *point;
    uint64_t numPoints = 0;
    bool flag;
    double tmp;
    uint64_t ulim = X.getPartitionSize(me);

    for (uint64_t i = 0; i < ulim; i++) {
        if (i == mediod->idx || !X.rowUnclusteredFromPartition(i, me))
            continue;

        point = X.getRowFromPartition(i, me);
        flag = true;

        for (auto j : subspace.itemset) {
            tmp = fabs(point->cells[j] - mediod->cells[j]);
            if (tmp > X.getSettings().width) {  // skip partition if not within width
                flag = false;
                break;
            }
        }

        if (!flag)
            continue;

        point->clusterMembership = clusterNum;
        point->clusMediod = mediod->id;
        X.decNumUnclustered();
        numPoints++;

        // no need to continue looking in the dataset if the number of points is equal to the support
        // no more points will be in range of cluster
        if (numPoints >= subspace.count)
            break;
    }

    return numPoints;
}

bool SharedSubspace::compareSubspaces(DimensionSet &subspace1, DimensionSet &subspace2) {
    double coef = 1 / _parameters.beta;

    if (subspace1.itemset.empty() || subspace1.count == 0)
        return false; // no dimensions automatically disqualifies centriod from being cluster center

    auto deltaDim = subspace1.itemset.size() - subspace2.itemset.size();

    if (deltaDim == 0)
        return subspace1.count > subspace2.count;
    else if (deltaDim < 0)
        return subspace1.count > (subspace2.count * pow(coef, (deltaDim)));
    else
        return subspace2.count < (subspace1.count * pow(coef, (deltaDim)));
}

void SharedSubspace::printCentroid(DimensionSet centroid) {
    std::cout << "Best Centroid: " << std::endl;
    std::cout << "  Dimensions:";
    for (const auto &i : centroid.itemset)
        std::cout << " " << i;
    std::cout << std::endl;
    std::cout << "  count: " << centroid.count << std::endl;
    std::cout << "  mu: " << centroid.mu << std::endl;
    std::cout << "  numPoints: " << centroid.numPoints << std::endl;
}