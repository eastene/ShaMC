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

typedef uint8_t PartitionID;

typedef std::vector<std::string> Header;
typedef uint64_t RowIndex;

/*
 *  Row of data set, a single sample or transaction
 */
struct Row {
    std::string id;
    RowIndex idx;
    std::vector<double> cells;
    int clusterMembership;
};
/*
 * constructs of multiple Rows
 */
typedef std::vector<Row*> MultiRow;
typedef std::unordered_map<RowIndex, Row*> MultiRowMap;

struct Partition {
    uint64_t start;
    uint64_t end;
};

class SharedDataset {
private:

    // dataset meta-info
    std::string path;
    std::string name;
    char delimiter;

    // shared implementation meta-info
    uint16_t num_threads;
    uint64_t bufferBytes;
    std::vector<uint64_t> row2byte;

    // dataset
    Header header;
    bool hasIndex;
    std::pair<uint64_t, uint64_t> inMemRange; // range of rows in memory
    MultiRow inMemBuffer;  // may hold entire dataset if small enough
    std::pair<RowIndex, int> _shape = std::make_pair(NULL, NULL);

    // auxilliary functions
    void read2Buffer(RowIndex startRow);

public:

    explicit SharedDataset(std::string &path,
                           uint16_t num_threads = 1,
                           char delimiter = ',',
                           bool header = true,              // 1st row is header
                           bool index = true,               // 1st column is index
                           uint64_t bufferBytes = 10485760);

    std::pair<RowIndex, int> shape() { return _shape; }

    Row* getRow(RowIndex index);

    Row* getRowAsynch(RowIndex index);

    Row* getRowFromPartition(RowIndex index, PartitionID paritionID);

    Row* getRowFromPartitionAsynch(RowIndex index, PartitionID paritionID);

    uint32_t getPartitionSize(PartitionID partitionID);

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
