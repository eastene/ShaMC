//
// Created by evan on 11/19/18.
//

#ifndef SHAMC_PROCESSSUBSPACE_HPP
#define SHAMC_PROCESSSUBSPACE_HPP


#include <cstdint>
#include <vector>
#include "ProcessTransactions.hpp"

typedef int Count;

struct DimensionSet {
    Dimension item;
    Count count;
    Count size;
};

class SharedSubspace {
private:
    Count _setcount;  // total number of itemsets
    Count _itemNumber;
    Count _minSupport;

    std::vector<DimensionSet*> _items;
public:

    SharedSubspace(Count minSupport, Count itemNumber) : _minSupport{minSupport},
                                                         _itemNumber{itemNumber},
                                                         _setcount{0} {};

    void addDimensionSet(int item, int count, int size);

    void addDimensionSet(int *items, int length, int level, int count, int size);

    const Count getSetCount() { return _setcount; }
};


#endif //SHAMC_PROCESSSUBSPACE_HPP
