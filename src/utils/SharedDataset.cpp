//
// Created by evan on 10/25/18.
//

#include <iostream>
#include <cmath>
#include <random>
#include "../../headers/utils/SharedDataset.hpp"

SharedDataset::SharedDataset(SharedSettings &parameters) {
    /*
     * Gathers metadata needed to maintain and distribute a shared dataset among threads accessing it concurrently.
     */

    std::string line;
    std::string column;

    // store data file meta-info
    this->path = parameters.dataPath;
    this->name = path.substr(path.rfind('/') + 1, path.size()); // TODO: make more portable (windows separator)
    this->delimiter = parameters.delimiter;
    this->hasIndex = parameters.index;
    this->parameters = parameters;

    this->num_threads = parameters.nThreads;

    std::ifstream datastream;

    // check for file and get file size in bytes
    datastream.open(path, std::ifstream::ate | std::ifstream::binary);
    if (!datastream) {
        throw FileNotFoundException(this->name);
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

    // get 1st row's byte value
    this->row2byte.push_back(datastream.tellg());
    datastream.close();
    readRows();

    this->rowsPerThread = this->row2byte.size() / this->num_threads;
    this->extraRows = this->row2byte.size() % this->num_threads;
    for (int p = 0; p < this->num_threads - 1; p++)
        this->partitions.emplace_back(std::make_pair(this->rowsPerThread * p, this->rowsPerThread * (p + 1)));

    this->partitions.emplace_back(std::make_pair(this->rowsPerThread * (this->num_threads - 1), this->row2byte.size() - 1));

    this->_shape = std::make_pair(this->row2byte.size(), this->header.size());
}

bool SharedDataset::readRows() {
    std::string line;
    std::string column;

    std::ifstream datastream(this->path);

    datastream.seekg(this->row2byte[0], std::ifstream::beg);

    // read in data to buffer until full or out of data
    uint64_t bytes_read = 0;
    this->inMemRange.first = 0;
    this->inMemRange.second = 0;

    while (!datastream.eof()) {
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
            } else {
                row->id = std::to_string(this->inMemRange.second);
            }
            row->cells.push_back(std::stod(column));
            row->idx = this->inMemRange.second;
            bytes_read += sizeof(double);
        }

        this->row2byte.emplace_back(datastream.tellg());
        this->inMemBuffer.push_back(row);
        this->inMemRange.second++;

        if(line.empty()){
            this->row2byte.pop_back();
            this->inMemRange.second--;
        }
    }
    datastream.close();
    this->row2byte.pop_back(); // last element will be end of last row

    return true;
}

bool SharedDataset::to_csv() {
    std::stringstream ss;
    std::ofstream out(parameters.resultPath);

    for (auto &col : header){
        ss << col << parameters.delimiter;
    }
    ss.seekp(-1, ss.cur);
    ss.put('\n');

    out << ss.str();

    for (auto &row : inMemBuffer) {
        ss.str(std::string());
        ss << row->id << parameters.delimiter;
        for (auto &cell : row->cells)
            ss << cell << parameters.delimiter;
        ss << row->clusterMembership;
        ss.put('\n');

        out << ss.str();
    }

    out.close();
    return true;
}

Row *SharedDataset::getRow(RowIndex index) {
/*
    if (index > (this->rowsPerThread * (this->num_threads - 1)))
    {
        uint64_t offset = index - ((this->num_threads - 1) * this->rowsPerThread);
        return this->buffers[(this->num_threads - 1)].getRow(offset);
    }
*/
    return this->inMemBuffer[index];
}

Row *SharedDataset::getRowAsynch(RowIndex index) {
    // TODO: read from disk if needed
    return this->inMemBuffer[index];
}

Row *SharedDataset::getRowFromPartition(RowIndex index, PartitionID paritionID) {
    RowIndex partInd = this->partitions[paritionID].first + index;
    return getRow(partInd);
}

Row *SharedDataset::getRowFromPartitionAsynch(RowIndex index, PartitionID paritionID) {
    // TODO: read from disk if needed
    RowIndex partInd = this->partitions[paritionID].first + index;
    return getRow(partInd);
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

    return this->partitions[partitionID].second - this->partitions[partitionID].first;
}

void SharedDataset::printMetaInfo() {
    std::cout << "Shared Dataset: " << this->name << std::endl;
    std::cout << "  Full Path: " << this->path << std::endl;
    std::cout << "  Has Header: " << !this->header.empty() << std::endl;
    std::cout << "  Has Index: " << this->hasIndex << std::endl;
    std::cout << "  Number of Columns: " << this->header.size() << std::endl;
    std::cout << "  Number of Rows:  " << this->row2byte.size() << std::endl;
    std::cout << "  Rows in Memory:  " << this->inMemRange.first + 1 << " - " << this->inMemRange.second << std::endl;
    std::cout << "  Threads Sharing: " << this->num_threads << std::endl;
}

void SharedDataset::printSummaryStats() {
    std::unordered_map<int, int> clusters;

    for (auto &row : inMemBuffer) {
        clusters[row->clusterMembership]++;
    }

    std::cout << std::endl << "///////////////////////////////////////" << std::endl;
    std::cout << "*************   Summary   *************" << std::endl;
    std::cout << "///////////////////////////////////////" << std::endl;

    if (clusters.empty()){
        std::cout << "No clusters found!" << std::endl;
        return;
    }

    std::cout << "Number of clusters found: " << clusters.size() << std::endl;
    int i = 0;
    for (auto &clus : clusters) {
        std::cout << "Cluster " << i++ << ": " << std::endl;
        std::cout << "  Number of points: " << clus.second << std::endl;
        //std::cout << "  Cluster Mediod: " << clus.mediodID << std::endl;
    }
}

void SharedDataset::repartition(uint16_t nThreads) {
    this->num_threads = nThreads;
    this->parameters.nThreads = nThreads;
    this->partitions.clear();

    this->rowsPerThread = this->num_threads > 0 ? this->row2byte.size() / this->num_threads : this->row2byte.size();
    this->extraRows = this->num_threads > 0 ? this->row2byte.size() % this->num_threads : 0;
    for (int p = 0; p < this->num_threads - 1; p++)
        this->partitions.emplace_back(std::make_pair(this->rowsPerThread * p, this->rowsPerThread * (p + 1) - 1));

    this->partitions.emplace_back(std::make_pair(this->rowsPerThread * (this->num_threads - 1), this->row2byte.size() - 1));

}