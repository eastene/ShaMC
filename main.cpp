#include <iostream>
#include "headers/utils/SharedDataset.hpp"
int main() {
    std::string path = "/home/evan/CLionProjects/ShaMC/data/soma_ae.csv";
    SharedDataset X(path);
    X.printMetaInfo();
    return 0;
}