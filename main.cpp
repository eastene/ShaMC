#include <iostream>
#include "headers/utils/SharedDataset.hpp"
#include "headers/utils/SharedSettings.hpp"
#include "headers/cluster/ShaMC.hpp"
int main() {
    std::string path = "/Users/evan/CLionProjects/ShaMC/data/soma_ae.csv";
    SharedDataset X(path);
    X.printMetaInfo();

    SharedSettings ss;
    ss.maxiter = 2;
    ShaMC mc(ss);
    mc.fit(X);

    return 0;
}