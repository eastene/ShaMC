#include <omp.h>
#include <iostream>
#include "headers/utils/SharedDataset.hpp"
#include "headers/utils/SharedSettings.hpp"
#include "headers/cluster/ShaMC.hpp"

int main() {
    std::string path = "/home/evan/CLionProjects/ShaMC/data/soma_PC3edDataset X(path, ss);
    X.printMetaInfo();

    ShaMC mc(ss);
    mc.fit(X);

    return 0;
}