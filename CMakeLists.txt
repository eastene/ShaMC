cmake_minimum_required(VERSION 3.12)
project(ShaMC)

set(CMAKE_CXX_STANDARD 11)

include(FindOpenMP)
if (OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

add_executable(ShaMC main.cpp
        src/utils/SharedDataset.cpp
        headers/utils/SharedDataset.hpp
        headers/utils/SharedSettings.hpp
        src/utils/ProcessDataBuffer.cpp
        headers/utils/ProcessDataBuffer.hpp
        headers/utils/PortMacros.hpp
        src/cluster/ShaMC.cpp
        headers/cluster/ShaMC.hpp
        headers/utils/logger.h
        src/ShaFEM/DataObject.cpp
        headers/ShaFEM/DataObject.h
        src/utils/ProcessTransactions.cpp
        headers/utils/ProcessTransactions.hpp
        src/ShaFEM/FPM_modified.cpp
        headers/ShaFEM/FPM_modified.hpp
        src/ShaFEM/buffer.cpp
        headers/ShaFEM/buffer.h)