//
// Created by evan on 10/25/18.
//

#include <iostream>
#include <cmath>
#include <random>
#include "../../headers/utils/SharedDataset.hpp"

SharedDataset::SharedDataset(std::string &path, SharedSettings &parameters) {
    /*
     * Gathers metadata needed to maintain and distribute a shared dataset among threads accessing it concurrently.
     */

    std::string line;
    std::string column;

    // store data file meta-info
    this->path = path;
    this->name = path.substr(path.rfind('/') + 1, path.size()); // TODO: make more portable (windows separator)
    this->delimiter = parameters.delimiter;
    this->hasIndex = parameters.index;
    this->parameters = parameters;

    this->num_threads = parameters.nThreads;

    std::ifstream datastream;

    // check for file and get file size in bytes
    datastream.open(path, std::ifstream::ate | std::ifstream::binary);
    if (!datastream) {
        throw FileNotFoundException(this->name); // TODO make more detailed exception
    }
    datastream.close();

    // read in header, get attribute names
    if (parameters.header) {
        datastream.open(path);
        std::getline(datastream, line);
        std::stringstream ss(line);
        while (std::getline(ss, column, this->delimiter)) {
            this->header.push_back(column);
        }
    }

    // find the beginning byte values of each row
    while (std::getline(datastream, line) && !datastream.eof())
        this->row2byte.push_back(datastream.tellg());

    this->rowsPerThread = this->row2byte.size() / this->num_threads;
    this->_shape = std::make_pair(this->row2byte.size(), this->header.size());

    for (int i = 0; i < this->num_threads; i++)
        this->buffers.emplace_back(
                ProcessDataBuffer(parameters.bufferBytes,
                                  this->row2byte[i * this->rowsPerThread],
                                  this->rowsPerThread,
                                  this->path,
                                  delimiter,
                                  hasIndex));

    for (auto &buffer : this->buffers)
        buffer.readRows();
}

Row *SharedDataset::getRow(RowIndex index) {
    PartitionID pid = index / this->rowsPerThread;
    uint64_t offset = index % this->rowsPerThread;

    return this->buffers[pid].getRow(offset);
}

Row *SharedDataset::getRowAsynch(RowIndex index) {
    PartitionID pid = index / this->rowsPerThread;
    uint64_t offset = index % this->rowsPerThread;

    return this->buffers[pid].getRowAsynch(offset);
}

Row *SharedDataset::getRowFromPartition(RowIndex index, PartitionID paritionID) {
    return this->buffers[paritionID].getRow(index);
}

Row *SharedDataset::getRowFromPartitionAsynch(RowIndex index, PartitionID paritionID) {
    return this->buffers[paritionID].getRowAsynch(index);
}

MultiRowMap SharedDataset::pickMediodsRandom(uint16_t n) {
    MultiRowMap _mediods(n);

    // random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned long> dis(0, _shape.first - 1);
    RowIndex i;
    RowIndex chosen = 0;

    // randomly choose n mediods
    while (chosen < n) {
        i = dis(gen);
        if (_mediods.find(i) == _mediods.end()) {
            _mediods[i] = getRow(i);
            chosen++;
        }
    }

    return _mediods;
}

uint64_t SharedDataset::getPartitionSize(PartitionID partitionID) {
    /*
     *  Returns the range of bytes of the dataset assigned to a given partition
     */

    return _shape.first >= (partitionID + 1) * this->rowsPerThread ? this->rowsPerThread : _shape.first - (partitionID *
                                                                                                           this->rowsPerThread);
}

void SharedDataset::printMetaInfo() {
    int buff = 0;
    std::cout << "Shared Dataset: " << this->name << std::endl;
    std::cout << "  Full Path: " << this->path << std::endl;
    std::cout << "  Has Header: " << !this->header.empty() << std::endl;
    std::cout << "  Has Index: " << this->hasIndex << std::endl;
    std::cout << "  Number of Columns: " << this->header.size() << std::endl;
    std::cout << "  Number of Rows:  " << this->row2byte.size() << std::endl;
    std::cout << "  Rows in Memory:  " << std::endl;
    for (auto &buffer : this->buffers) {
        std::cout << "    Thread Buffer #" << std::to_string(buff) << ": "
                  << buff * this->rowsPerThread + buffer.getMemRange().first << " - "
                  << buff * this->rowsPerThread + buffer.getMemRange().second - 1 << std::endl;
        buff++;
    }

    std::cout << "  Threads Sharing: " << this->num_threads << std::endl;
}