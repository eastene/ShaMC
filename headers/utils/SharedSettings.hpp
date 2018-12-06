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
    double alpha = 0.005;
    double beta = 0.25;
    int mediods = 1 * (int)ceil(2 / alpha);
    int maxAttempts = 2;
    uint32_t maxiter = 1000;
    bool saveResult = true;
    bool inPlace = true;   // if true add cluster labels to existing dataset, if false, return copy of X with labels
    int nThreads = 1;      // number of threads to use in computation
    std::string dataPath = "";
    std::string resultPath = "";
    char delimiter = ',';
    bool header = true;               // 1st row is header
    bool index = true;                // 1st column is index
    uint64_t bufferBytes = 10485760;  // 10MiB
    int minsupport = 1;

};


#endif //SHAMC_SHAREDSETTINGS_HPP
