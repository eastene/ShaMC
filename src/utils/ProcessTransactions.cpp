//
// Created by evan on 11/16/18.
//

#include <cmath>

#include "../../headers/utils/ProcessTransactions.hpp"

typedef uint16_t DimensionItem;

void ProcessTransactions::ProcessTransactions::buildTransactionsPar(RowIndex centroidID, SharedDataset &X, PartitionID me) {
    Row *centroid = X.getRow(centroidID);
    Row *point;

    for (uint64_t i = 0; i < X.getPartitionSize(me); i++) {
        if (i == centroid->idx)
            continue;

        point = X.getRowFromPartition(i, me);
        for (DimensionItem j = 0; j < point->cells.size(); j++) {
            if (fabs(point->cells[j] - centroid->cells[j]) <= X.getSettings().width) {
                this->transactions[i].emplace_back(j);
                this->counts.count(j);
            }
        }
        numTransactions++;
    }
}