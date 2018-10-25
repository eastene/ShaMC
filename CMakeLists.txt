cmake_minimum_required(VERSION 3.12)
project(ShaMC)

set(CMAKE_CXX_STANDARD 11)

add_executable(ShaMC main.cpp src/utils/SharedDataset.cpp headers/utils/SharedDataset.hpp headers/utils/SharedSettings.hpp headers/utils/PortMacros.hpp src/cluster/ShaMC.cpp headers/cluster/ShaMC.hpp headers/utils/Row.hpp)