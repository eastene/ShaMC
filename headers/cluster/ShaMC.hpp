//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_SHAMC_HPP
#define SHAMC_SHAMC_HPP


#include <vector>
#include "../utils/SharedDataset.hpp"
#include "../utils/SharedSettings.hpp"
#include "../utils/Row.hpp"

typedef std::vector<Row> MultiRow;

class ShaMC {
private:
    MultiRow centroids;
    SharedSettings parameters;

    MultiRow pickCentroidsRandom(SharedDataset &X, unsigned int n);

public:
    ShaMC();
    // find mediods using subspace clustering, overwrites any existing mediods
    void fit(SharedDataset& X);
    // predict labels using mediods found after fitting
    SharedDataset predict(SharedDataset& X);
    // perform fitting and prediction in single step (useful for training set)
    SharedDataset fit_predict(SharedDataset& X);
};


#endif //SHAMC_SHAMC_HPP
