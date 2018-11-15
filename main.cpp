#include <iostream>
#include "headers/utils/SharedDataset.hpp"
#include "headers/utils/SharedSettings.hpp"
#include "headers/cluster/ShaMC.hpp"
int main() {
    std::string path = "/home/evan/CLionProjects/ShaMC/data/soma_ae.csv";
    SharedDataset X(path, 4);
    X.printMetaInfo();

    omp_set_num_threads(4);

    SharedSettings ss;
    ss.maxiter = 2;
    ss.nThreads = 4;
    ShaMC mc(ss);
    mc.fit(X);

    return 0;
}