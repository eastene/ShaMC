//
// Created by evan on 11/19/18.
//

#ifndef SHAMC_PROCESSSUBSPACE_HPP
#define SHAMC_PROCESSSUBSPACE_HPP


#include <cstdint>
#include <vector>
#include <set>
#include "ProcessTransactions.hpp"

typedef int Count;

struct DimensionSet {
    std::string itemset;
    std::string count;
    Count size;
};

class SharedSubspace {
private:
    Count _itemNumber;
    Count _minSupport;
    SharedSettings _parameters;
    IntToString* _lookup;

    std::vector<DimensionSet *> _items;
public:

    SharedSubspace(Count minSupport, Count itemNumber, SharedSettings &parameters) : _minSupport{minSupport},
                                                                                    _itemNumber{itemNumber},
                                                                                    _parameters{parameters} {_lookup = new IntToString(_parameters.minsupport);}

    void addDimensionSet(int item, int count, int size);

    void addDimensionSet(int *items, int length, int level, int count, int size);

    const Count getSetCount() { return _items.size(); }

    const std::vector<DimensionSet *> getSubspaces() { return _items; }
};


#endif //SHAMC_PROCESSSUBSPACE_HPP
