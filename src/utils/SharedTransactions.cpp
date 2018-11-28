//
// Created by evan on 11/16/18.
//

#include <cmath>
#include <cstring>
#include <iostream>

#include "../../headers/utils/SharedTransactions.hpp"

void SharedTransactions::buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me) {
    Row *centroid = X.getRow(centroidID);
    Row *point;
    bool flag; // false if no items added for a transaction

    _numTransactions = 0;
    _numItems = 0;

    _reduced = false;

    for (uint64_t i = 0; i < X.getPartitionSize(me); i++) {
        flag = false;
        if (i == centroid->idx)
            continue;

        point = X.getRowFromPartition(i, me);
        for (Dimension j = 0; j < point->cells.size(); j++) {
            if (fabs(point->cells[j] - centroid->cells[j]) <= X.getSettings().width) {
                *_parStreams[me] << std::to_string(j) << " ";
                flag = true;
                _numItems++;
            }
        }
        if (flag) {
            *_parStreams[me] << "\n";
            _numTransactions++;
        }
    }
}

void SharedTransactions::reduce() {
    if (!_reduced) {
        delete _transactions;
        _transactions = new std::stringstream;
        for (const auto &stream : _parStreams) {
            if(!stream->eof()){
                *(_transactions) << stream->str();
                stream->str(std::string());
            }
        }
        _reduced = true;
    }
}