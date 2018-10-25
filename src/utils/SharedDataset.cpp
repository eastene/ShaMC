//
// Created by evan on 10/25/18.
//

#include <thread>
#include <fstream>
#include "../../headers/utils/SharedDataset.hpp"

SharedDataset::SharedDataset(std::string& path, char delimiter=',') {
    std::stringstream headers;
    std::string column;
    // check for file and get file size in bytes
    std::ifstream datastream(path, std::ifstream::ate | std::ifstream::binary);
    if (!datastream) {
        throw exception& e; // TODO make more detailed exception
    }
    this->rawFileSize = datastream.tellg();
    this->path = path;
    datastream.close()

    datastream.open(path)
    std::getline(datastream, headers);
    while(std::getline(headers, column, delimiter) != std::string::npos){
        this->header.push_back(column);
    }
}

Row SharedDataset::getRowAsynch(RowIndex index) {
    std::thread t([&]() {

    });
}