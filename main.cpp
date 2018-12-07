#include <omp.h>
#include <iostream>
#include "headers/utils/SharedDataset.hpp"
#include "headers/utils/SharedSettings.hpp"
#include "headers/cluster/ShaMC.hpp"

int main() {

    int num_threads = 2;

    omp_set_num_threads(num_threads);

    std::string path = "/home/evan/CLionProjects/ShaMC/data/synthetic_5_100_1000.csv";
    SharedSettings ss;
    ss.nThreads = num_threads;
    ss.width = 20;
    ss.maxiter = 1000;
    ss.dataPath = path;
    ss.resultPath = "/home/evan/CLionProjects/ShaMC/data/synthetic_5_100_1000_clus.csv";

    SharedDataset X(ss);
    X.printMetaInfo();

    ShaMC mc(ss);
    mc.fit(X);
    X.to_csv();
    return 0;
}