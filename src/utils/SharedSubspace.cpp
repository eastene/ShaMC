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
    DimensionSet subspace;
    subspace.mediodID = mediodID;

    subspace.count = 0;
    subspace.itemset = tmpDimSet;
    subspace.mu = 0.0;
    subspace.numPoints = 0;

    if (!dimensionSet || dimensionSet->str().empty())
        return subspace;

    int i = -2;
    dimensionSet->seekg(i, dimensionSet->end);
    while (dimensionSet->peek() != '\n') {
        dimensionSet->seekg(i--, dimensionSet->end);
    }
    // consume newline
    dimensionSet->get();
    std::getline(*dimensionSet, line);

    std::stringstream ss(line);
    while (std::getline(ss, token, ' ')) {
        if (token[0] == '(')
            support = std::stoi(token.substr(1, token.size() - 2)); // support in parens
        else
            tmpDimSet.push_back(std::stoi(token));
    }

    double mu = support * pow((1 / _parameters.beta), tmpDimSet.size());

    subspace.count = support;
    subspace.itemset = tmpDimSet;
    subspace.mu = mu;

    return subspace;
}

uint64_t SharedSubspace::clusterPar(SharedDataset &X, PartitionID me, int clusterNum, DimensionSet &subspace) {
    Row *mediod = X.getRow(subspace.mediodID);
    Row *point;
    uint64_t numPoints = 0;
    bool flag;
    uint64_t ulim = X.getPartitionSize(me);

    for (uint64_t i = 0; i < ulim; i++) {
        if (i == mediod->idx || !X.rowUnclusteredFromPartition(i, me))
            continue;

        point = X.getRowFromPartition(i, me);
        flag = true;

        for (auto j : subspace.itemset) {
            if (fabs(point->cells[j] - mediod->cells[j]) > X.getSettings().width) {  // skip partition if not within width
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