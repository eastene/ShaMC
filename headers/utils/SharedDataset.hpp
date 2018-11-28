//
// Created by evan on 10/25/18.
//

#ifndef SHAMC_SHAREDDATASET_HPP
#define SHAMC_SHAREDDATASET_HPP


#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "ProcessDataBuffer.hpp"
#include "SharedSettings.hpp"

typedef int PartitionID;

typedef std::vector<std::string> Header;


/*
 *  Implements a single data-management structure shared by all processes
 *  any locking/sychronization from a data perspective should be implmented here
 */
class SharedDataset {
private:

    // dataset meta-info
    std::string path;
    std::string name;
    char delimiter;
    SharedSettings parameters;

    // shared implementation meta-info
    uint16_t num_threads;
    uint64_t rowsPerThread;
    std::vector<uint64_t> row2byte;

    // dataset
    Header header;
    bool hasIndex;
    std::vector<ProcessDataBuffer> buffers;  // may hold entire dataset if small enough
    std::pair<RowIndex, int> _shape = std::make_pair(NULL, NULL);

public:

    explicit SharedDataset(std::string &path, SharedSettings &parameters);

    std::pair<RowIndex, int> shape() { return _shape; }

    Row* getRow(RowIndex index);

    Row* getRowAsynch(RowIndex index);

    Row* getRowFromPartition(RowIndex index, PartitionID paritionID);

    Row* getRowFromPartitionAsynch(RowIndex index, PartitionID paritionID);

    MultiRowMap pickMediodsRandom(uint16_t n);

    uint64_t getPartitionSize(PartitionID partitionID);

    const SharedSettings getSettings() {return parameters;}

    void printMetaInfo();
};


/*
 * Shared dataset exceptions
 */

class FileNotFoundException : public std::exception {
private:
    std::string message;
public:
    explicit FileNotFoundException(std::string &filename) { message = filename + " not found."; }

    const char *what() { return message.c_str(); }
};

#endif //SHAMC_SHAREDDATASET_HPP
