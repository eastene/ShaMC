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
    uint64_t extraRows;  // when num_threads does not evenly divide #rows (used only by last thread)
    std::vector<uint64_t> row2byte;
    std::vector<std::pair<uint64_t, uint64_t>> partitions;

    // dataset
    Header header;
    bool hasIndex;
    MultiRow inMemBuffer;  // may hold entire dataset if small enough
    std::pair<uint64_t, uint64_t> inMemRange; // range of rows in memory
    std::pair<RowIndex, int> _shape = std::make_pair(NULL, NULL);

public:

    explicit SharedDataset(std::string &path, SharedSettings &parameters);

    bool readRows();

    std::pair<RowIndex, int> shape() { return _shape; }

    Row* getRow(RowIndex index);

    Row* getRowAsynch(RowIndex index);

    Row* getRowFromPartition(RowIndex index, PartitionID paritionID);

    Row* getRowFromPartitionAsynch(RowIndex index, PartitionID paritionID);

    MultiRowMap pickMediodsRandom(uint16_t n);

    uint64_t getPartitionSize(PartitionID partitionID);

    const SharedSettings getSettings() {return parameters;}

    void printMetaInfo();

    void repartition(uint16_t nThreads);
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
