//
// Created by evan on 11/15/18.
//

#ifndef SHAMC_PROCESSDATABUFFER_HPP
#define SHAMC_PROCESSDATABUFFER_HPP

#include <string>
#include <vector>
#include <unordered_map>

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
typedef std::vector<Row *> MultiRow;
typedef std::unordered_map<RowIndex, Row *> MultiRowMap;

/*
 *  Implements a single data buffer for a given process
 */
class ProcessDataBuffer {
private:
    std::string path;
    char delimiter;
    bool hasIndex;
    MultiRow inMemBuffer;  // may hold entire dataset if small enough
    std::pair<uint64_t, uint64_t> inMemRange; // range of rows in memory
    uint64_t bufferBytes;
    uint64_t numRows;
    long nextByte;  // next byte to read (if able)
    bool moreData;      // more data to read

public:
    ProcessDataBuffer(uint64_t bufferBytes, uint64_t startByte, uint64_t numRows, std::string &path, char delimiter,
                      bool hasIndex);

    bool readRows();

    Row *getRow(RowIndex index);

    Row *getRowAsynch(RowIndex index);
};

/*
 * Data Buffer Exceptions
 */

class OutOfRangeException : public std::exception {
public:
    OutOfRangeException() = default;

    const char *what() { return "Index out of range of data buffer."; }
};


#endif //SHAMC_PROCESSDATABUFFER_HPP
