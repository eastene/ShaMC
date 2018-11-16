#include <iostream>
#include "headers/utils/SharedDataset.hpp"
#include "headers/utils/SharedSettings.hpp"
#include "headers/cluster/ShaMC.hpp"
int main() {
    std::string path = "/home/evan/CLionProjects/ShaMC/data/soma_ae.csv";

    int num_threads = 1;
    omp_set_num_threads(num_threads);

    SharedSettings ss;
    ss.maxiter = 2;
    ss.nThreads = num_threads;

    SharedDataset X(path, ss);
    X.printMetaInfo();

    ShaMC mc(ss);
    mc.fit(X);

    return 0;
}