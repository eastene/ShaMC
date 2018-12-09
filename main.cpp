#include <omp.h>
#include <iostream>
#include "headers/utils/SharedDataset.hpp"
#include "headers/utils/SharedSettings.hpp"
#include "headers/cluster/ShaMC.hpp"

int main(int argc, char** argv) {

    if (argc < 4) {
        std::cout << "usage: ./ShaMC num_threads input_path output_path [width] [alpha] [beta]" << std::endl;
    }

    int num_threads = std::stoi(argv[1]);

    omp_set_num_threads(num_threads);

    SharedSettings ss;
    ss.nThreads = num_threads;
    ss.width = argc == 5 ? std::stoi(argv[4]) : 30;
    ss.alpha = argc == 6 ? std::stoi(argv[5]) : 0.1;
    ss.beta = argc == 7 ? std::stoi(argv[6]) : 0.25;
    ss.maxiter = 1000;
    ss.dataPath = argv[2];
    ss.resultPath = argv[3];

    SharedDataset X(ss);
    X.printMetaInfo();

    ShaMC mc(ss);
    mc.fit(X);
    X.to_csv();

    return 0;
}