//
// Created by evan on 11/15/18.
//

#include <fstream>
#include <sstream>
#include "../../headers/utils/ProcessDataBuffer.hpp"

ProcessDataBuffer::ProcessDataBuffer(uint64_t bufferBytes, uint64_t startByte, uint64_t numRows, std::string &path,
                                     char delimiter, bool hasIndex) {
    this->path = path;
    this->delimiter = delimiter;
    this->hasIndex = hasIndex;
    this->nextByte = startByte;
    this->numRows = numRows;
    this->inMemRange.second = 0;
    this->moreData = true;
}

bool ProcessDataBuffer::readRows() {
    std::string line;
    std::string column;

    if (!this->moreData)
        return false;

    std::ifstream datastream(this->path);

    datastream.seekg(this->nextByte, std::ifstream::beg);

    // read in data to buffer until full or out of data
    uint64_t bytes_read = 0;
    this->inMemRange.first = this->inMemRange.second;
    while (!datastream.eof() && bytes_read <= this->bufferBytes) {
        std::getline(datastream, line);
        std::stringstream ss(line);
        Row *row = new Row;
        bool readIndex = this->hasIndex;
        while (std::getline(ss, column, this->delimiter)) {
            if (readIndex) {
                // if the first column is an index, read into id field of row, then skip
                row->id = column;
                readIndex = false;
                continue;
            }
            row->cells.push_back(std::stod(column));
            row->idx = this->inMemRange.second;
            row->clusterMembership = 0;
            bytes_read += sizeof(double);
        }

        if (bytes_read <= this->bufferBytes) {
            this->inMemBuffer.push_back(row);
            this->inMemRange.second++;
        }

        if (this->inMemRange.second >= this->numRows) {
            this->moreData = false;
            break;
        }
    }

    this->nextByte = datastream.tellg();
    datastream.close();

    return true;
}

Row *ProcessDataBuffer::getRow(RowIndex index) {
    if (index >= this->inMemRange.first && index < this->inMemRange.second)
        return this->inMemBuffer[index];
    if (readRows())
        return this->inMemBuffer[index];
    else
        throw OutOfRangeException();
}


Row *ProcessDataBuffer::getRowAsynch(RowIndex index) {
    // TODO: implement correctly
    // asynchronously fectch the row, filling the buffer with new rows
    //auto f = [&]() { read2Buffer(this->row2byte[index]); };
    //std::thread t(f);

    //t.join();
    return new Row();
}