#include <omp.h>
#include <iostream>
#include "headers/utils/SharedDataset.hpp"
#include "headers/utils/SharedSettings.hpp"
#include "headers/cluster/ShaMC.hpp"

int main() {
    std::string path = "/home/evan/CLionProjects/ShaMC/data/soma_PC3.csv";

    int num_threads = 1;
    omp_set_num_threads(num_threads);

    SharedSettings ss;
    ss.maxiter = 1;
    ss.nThreads = num_threads;
    ss.width = 4;

    SharedDataset X(path, ss);
    X.printMetaInfo();

    ShaMC mc(ss);
    mc.fit(X);

    return 0;
}