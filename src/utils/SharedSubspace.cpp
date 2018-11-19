//
// Created by evan on 11/19/18.
//

#include "../../headers/utils/SharedSubspace.hpp"

void SharedSubspace::addDimensionSet(int item, int count, int size) {
    _items.push_back(new DimensionSet{item, count, size});
}

void SharedSubspace::addDimensionSet(int *items, int length, int level, int count, int size) {
    for (int i = 0; i < length; i++) {
        _items.push_back(new DimensionSet{items[i], count, size});
    }
}