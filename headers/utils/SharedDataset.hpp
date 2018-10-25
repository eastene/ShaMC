//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_SHAREDDATASET_HPP
#define SHAMC_SHAREDDATASET_HPP


#include <string>
#include "Row.hpp"

typedef std::vector<std::string> Header;
typedef uint64_t RowIndex;

class SharedDataset {
private:
    std::string path;
    std::string name;
    long rawFileSize;
    Header header;
    std::pair<RowIndex, int> _shape = std::make_pair(NULL, NULL);

public:

    explicit SharedDataset(std::string path);

    std::pair<int, int> shape(){return _shape;}
    Row getRowAsynch(RowIndex index);
};


#endif //SHAMC_SHAREDDATASET_HPP
