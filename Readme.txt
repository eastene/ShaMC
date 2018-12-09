ShaMC

Shared-memory MIMD MineClus for subspace clustering on high dimensional
datasets.

Usage:
./ShaMC num_threads input output [width] [alpha] [beta]

Arguments:
num_threads - Number of threads that will run clustering in parallel.

input - Input file, a comma delimited data file.

output - Output file, a copy of the input file with new column containing
 cluster number. A cluster label of -1 indicates an outlier

width - Width of subspace. Points within width distance from a centroid
along a given dimension are included in that subspace. Measured with
Manhattan distance. Higher values will generate larger subspaces, and thus
increase the running time. (Optional, default=30)

alpha - Sets minimum support (minimum points in a cluster) as alpha *
size of input data. Higher values will require larger clusters and thus
generate fewer clusters overall. (Optional, defualt=0.1)

beta - Sets proportionality of number of points to number of dimensions when
choosing the best subspace. Higher values will give greater priority to
number of dimensions. (Optional, default=0.25)

Building:

ShaMC is built and tested using C++11, OpenMP 3.1, and CMAKE 2.8.
ShaMC requires gcc-4.8.5 or higher or clang-1000.10.44.4 or higher.

To Build:
1. cmake CMakeLists.txt
2. make