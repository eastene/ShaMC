//
// Created by evan on 11/19/18.
//

#include <iostream>
#include "../../headers/utils/SharedSubspace.hpp"

void SharedSubspace::buildSubspace(std::stringstream *dimensionSet, RowIndex mediodID) {
    std::string line;
    std::string token;
    int support = 0;
    std::vector<int> tmpDimSet;
    double mu_best = 0.0;

    _centroid.mediodID = mediodID;

    if (!dimensionSet) {
        _centroid.count = support;
        _centroid.itemset = tmpDimSet;
        _centroid.mu = mu_best;
        return;
    }

    while (!std::getline(*dimensionSet, line, '\n').eof()) {
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
            _centroid.count = support;
            _centroid.itemset = tmpDimSet;
            _centroid.mu = mu;
            mu_best = mu;
        }

        tmpDimSet.clear();
    }

    return;
}

uint64_t SharedSubspace::clusterPar(SharedDataset &X, PartitionID me, int clusterNum) {
    Row *mediod = X.getRow(_centroid.mediodID);
    Row *point;
    int tally;
    uint64_t numPoints = 0;
    double dist = 0;

    for (uint64_t i = 0; i < X.getPartitionSize(me); i++) {
        if (i == mediod->idx)
            continue;

        point = X.getRowFromPartition(i, me);

        tally = 0;
        for (auto j : _centroid.itemset) {
            double tmp = fabs(point->cells[j] - mediod->cells[j]);
            if (tmp <= X.getSettings().width) {
                tally++;
                dist += tmp;
            }
        }

        if (tally == _centroid.itemset.size() && dist < point->closestDist) {
            point->clusterMembership = clusterNum;
            point->closestDist = dist;
            point->clusMediod = mediod->id;
            numPoints++;
        }
    }

    return numPoints;
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