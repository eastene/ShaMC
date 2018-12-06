#include <omp.h>
#include <iostream>
#include "headers/utils/SharedDataset.hpp"
#include "headers/utils/SharedSettings.hpp"
#include "headers/cluster/ShaMC.hpp"

int main() {

    int num_threads = 8;

    omp_set_num_threads(num_threads);

    std::string path = "/home/evan/CLionProjects/ShaMC/data/soma_PC5.csv";
    SharedSettings ss;
    ss.nThreads = num_threads;
    ss.width = 20;
    ss.maxiter = 2;
    ss.dataPath = path;
    ss.resultPath = "/home/evan/CLionProjects/ShaMC/data/soma_PC5_clus.csv";

    SharedDataset X(ss);
    X.printMetaInfo();

    ShaMC mc(ss);
    mc.fit(X);
    X.to_csv();
    return 0;
}