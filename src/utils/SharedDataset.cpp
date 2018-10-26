//
// Created by evan on 10/25/18.
//

#include <thread>
#include <fstream>
#include <sstream>
#include "../../headers/utils/SharedDataset.hpp"

SharedDataset::SharedDataset(std::string& path, uint16_t num_threads, char delimiter) {
    /*
     * Gathers metadata needed to maintain and distribute a shared dataset among threads accessing it concurrently.
     */
    std::string line;
    std::string column;

    this->num_threads = num_threads;

    // check for file and get file size in bytes
    std::ifstream datastream(path, std::ifstream::ate | std::ifstream::binary);
    if (!datastream) {
        throw FileNotFoundException(this->name); // TODO make more detailed exception
    }
    this->rawFileSize = datastream.tellg();
    this->path = path;
    datastream.close();

    datastream.open(path);
    std::getline(datastream, line);
    std::stringstream ss(line);
    while(std::getline(ss, column, delimiter)){
        this->header.push_back(column);
    }

}

Row SharedDataset::getRowAsynch(RowIndex index) {
    //std::thread t([&]() {

    //});
    return Row();
}

Partition SharedDataset::getPartition(unsigned int partitionID) {
    /*
     *  Returns the range of bytes of the dataset assigned to a given partition
     */
    Partition p {0,0};
    p.start = partitionID * bytesPerPartition;
    p.end = p.start + bytesPerPartition;

    return p;
}