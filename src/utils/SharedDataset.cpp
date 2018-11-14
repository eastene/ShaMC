//
// Created by evan on 10/25/18.
//

#include <iostream>
#include "../../headers/utils/SharedDataset.hpp"

void SharedDataset::read2Buffer(RowIndex startRow) {
    std::string line;
    std::string column;

    std::ifstream datastream(this->path);

    datastream.seekg(this->row2byte[startRow], std::ifstream::beg);

    // read in data to buffer until full or out of data
    uint64_t bytes_read = 0;
    this->inMemRange.first = startRow;
    this->inMemRange.second = startRow;
    while (!datastream.eof() && bytes_read <= bufferBytes) {
        std::getline(datastream, line);
        std::stringstream ss(line);
        Row *row = new Row;
        bool readIndex = this->hasIndex;
        while (std::getline(ss, column, delimiter)) {
            if (readIndex) {
                // if the first column is an index, read into id field of row, then skip
                row->id = column;
                readIndex = false;
                continue;
            }
            row->cells.push_back(std::stod(column));
            bytes_read += sizeof(double);
        }

        if (bytes_read <= bufferBytes) {
            this->inMemBuffer.push_back(row);
            this->inMemRange.second++;
        }
    }
}

SharedDataset::SharedDataset(std::string &path, uint16_t num_threads, char delimiter, bool header, bool index,
                             uint64_t bufferBytes) {
    /*
     * Gathers metadata needed to maintain and distribute a shared dataset among threads accessing it concurrently.
     */

    std::string line;
    std::string column;

    // store data file meta-info
    this->path = path;
    this->name = path.substr(path.rfind('/') + 1, path.size()); // TODO: make more portable (windows separator)
    this->delimiter = delimiter;
    this->hasIndex = index;

    this->num_threads = num_threads;
    this->bufferBytes = bufferBytes;  // default of 10 MiB

    std::ifstream datastream;

    // check for file and get file size in bytes
    datastream.open(path, std::ifstream::ate | std::ifstream::binary);
    if (!datastream) {
        throw FileNotFoundException(this->name); // TODO make more detailed exception
    }
    datastream.close();

    // read in header, get attribute names
    if (header) {
        datastream.open(path);
        std::getline(datastream, line);
        std::stringstream ss(line);
        while (std::getline(ss, column, this->delimiter)) {
            this->header.push_back(column);
        }
    }

    // find the beginning byte values of each row
    this->row2byte.push_back(datastream.tellg());
    while (std::getline(datastream, line))
        this->row2byte.push_back(datastream.tellg());
    read2Buffer(0);
}

Row* SharedDataset::getRow(RowIndex index) {
    if (index >= this->inMemRange.first && index <= this->inMemRange.second)
        return this->inMemBuffer[index];
    else {
        read2Buffer(this->row2byte[index]);
        return this->inMemBuffer[index];
    }
}

Row* SharedDataset::getRowAsynch(RowIndex index) {
    // if already in memory, simply fetch
    if (index >= this->inMemRange.first && index <= this->inMemRange.second)
        return this->inMemBuffer[index];

    // TODO: implement correctly
    // asynchronously fectch the row, filling the buffer with new rows
    //auto f = [&]() { read2Buffer(this->row2byte[index]); };
    //std::thread t(f);

    //t.join();
    return this->inMemBuffer[index];
}

uint32_t SharedDataset::getPartitionSize(PartitionID partitionID) {
    /*
     *  Returns the range of bytes of the dataset assigned to a given partition
     */

    Partition p{0, 0};
    uint64_t rowsPerThread = this->row2byte.size() / this->num_threads;
    p.start = partitionID * rowsPerThread;
    p.end = p.start + rowsPerThread;

    return p;
}

void SharedDataset::printMetaInfo() {
    std::cout << "Shared Dataset: " << this->name << std::endl;
    std::cout << "  Full Path: " << this->path << std::endl;
    std::cout << "  Has Header: " << !this->header.empty() << std::endl;
    std::cout << "  Has Index: " << this->hasIndex << std::endl;
    std::cout << "  Number of Columns: " << this->header.size() << std::endl;
    std::cout << "  Number of Rows:  " << this->row2byte.size() << std::endl;
    std::cout << "  Rows in Memory:  " << this->inMemRange.first << " - " << this->inMemRange.second - 1 << std::endl;
    std::cout << "  Threads Sharing: " << this->num_threads << std::endl;
}