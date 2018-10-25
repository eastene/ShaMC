//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_SHAREDSETTINGS_HPP
#define SHAMC_SHAREDSETTINGS_HPP

#include "PortMacros.hpp"
#include <string>
#include <cmath>

struct SharedSettings{
    double width = 2.5;
    double alpha = 0.5;
    double beta = 0.25;
    int mediods = 1 * (int)ceil(2 / alpha);
    int maxAttempts = 2;
    bool saveResult = true;
    bool inPlace = true;   // if true add cluster labels to existing dataset, if false, return copy of X with labels
    int nThreads = 1;      // number of threads to use in computation
    std::string dataPath = "";
    std::string resultPath = "";
};


#endif //SHAMC_SHAREDSETTINGS_HPP
