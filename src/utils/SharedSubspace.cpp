//
// Created by evan on 11/19/18.
//

#include <iostream>
#include "../../headers/utils/SharedSubspace.hpp"

void SharedSubspace::addDimensionSet(int item, int count, int size) {
    int len;
    char *buf = _lookup->convertItem(item, len);
    char *count_buf = _lookup->convertCount(count, len);

    _items.push_back(new DimensionSet{buf, count_buf, size});
}

void SharedSubspace::addDimensionSet(int *items, int length, int level, int count, int size) {
    do {
        addDimensionSet(items[level++], count, size);

        if (level < length) addDimensionSet(items, length, level, count, size + 1);
        else break;

    } while (true);
}