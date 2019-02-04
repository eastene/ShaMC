cmake_minimum_required(VERSION 2.8)
project(ShaMC)

set(CMAKE_CXX_STANDARD 11)

# Find OpenMP
if (APPLE AND USE_OpenMP)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(OpenMP_C "${CMAKE_C_COMPILER}")
        set(OpenMP_C_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
        set(OpenMP_C_LIB_NAMES "libomp" "libgomp" "libiomp5")
        set(OpenMP_libomp_LIBRARY ${OpenMP_C_LIB_NAMES})
        set(OpenMP_libgomp_LIBRARY ${OpenMP_C_LIB_NAMES})
        set(OpenMP_libiomp5_LIBRARY ${OpenMP_C_LIB_NAMES})
    endif ()
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(OpenMP_CXX "${CMAKE_CXX_COMPILER}")
        set(OpenMP_CXX_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
        set(OpenMP_CXX_LIB_NAMES "libomp" "libgomp" "libiomp5")
        set(OpenMP_libomp_LIBRARY ${OpenMP_CXX_LIB_NAMES})
        set(OpenMP_libgomp_LIBRARY ${OpenMP_CXX_LIB_NAMES})
        set(OpenMP_libiomp5_LIBRARY ${OpenMP_CXX_LIB_NAMES})
    endif ()
endif ()

if (USE_OpenMP)
    find_package(OpenMP REQUIRED)
endif (USE_OpenMP)

if (OPENMP_FOUND)
    include_directories("${OPENMP_INCLUDES}")
    link_directories("${OPENMP_LIBRARIES}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif (OPENMP_FOUND)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")  # for some reason not specified
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O")

add_executable(ShaMC main.cpp
        src/utils/SharedDataset.cpp headers/utils/SharedDataset.hpp
        headers/utils/SharedSettings.hpp
        headers/utils/PortMacros.hpp
        src/cluster/ShaMC.cpp headers/cluster/ShaMC.hpp
        headers/utils/logger.h
        src/utils/SharedTransactions.cpp headers/utils/SharedTransactions.hpp
        src/utils/SharedSubspace.cpp headers/utils/SharedSubspace.hpp
        ShaFEM-MEM/FPM.cpp ShaFEM-MEM/FPM.h
        ShaFEM-MEM/InputData.cpp ShaFEM-MEM/InputData.h
        ShaFEM-MEM/OutputData.cpp ShaFEM-MEM/OutputData.h
        ShaFEM-MEM/DataObject.cpp ShaFEM-MEM/DataObject.h
        ShaFEM-MEM/buffer.cpp ShaFEM-MEM/buffer.h)