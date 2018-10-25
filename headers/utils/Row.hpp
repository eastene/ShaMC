//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_ROW_HPP
#define SHAMC_ROW_HPP

#include <vector>
#include <string>

/*
 *  Row of data set, a single sample or transaction
 */
struct Row {
    int64_t pos;    // position within the file
    std::string id;
    std::vector<double> cells;
    int clusterMembership;
};

#endif //SHAMC_ROW_HPP
