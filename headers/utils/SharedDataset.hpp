//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_SHAREDDATASET_HPP
#define SHAMC_SHAREDDATASET_HPP


#include <string>
#include "Row.hpp"


typedef std::vector<std::string> Header;
typedef uint64_t RowIndex;

struct Partition {
    uint64_t start;
    uint64_t end;
};

class SharedDataset {
private:
    std::string path;
    std::string name;
    uint16_t num_threads;
    long rawFileSize;
    uint32_t rawRowSize;
    uint64_t bytesPerPartition;
    Header header;
    std::pair<RowIndex, int> _shape = std::make_pair(NULL, NULL);

public:

    explicit SharedDataset(std::string& path, uint16_t num_threads=1, char delimiter=',');

    std::pair<RowIndex, int> shape(){return _shape;}
    Row getRowAsynch(RowIndex index);
    Partition getPartition(unsigned int partitionID);
};


/*
 * Shared dataset exceptions
 */

class FileNotFoundException: public std::exception {
private:
    std::string message;
public:
    explicit FileNotFoundException(std::string& filename){message=filename + " not found.";}
    const char* what(){return message.c_str();}
};

#endif //SHAMC_SHAREDDATASET_HPP
