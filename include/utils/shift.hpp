#pragma once
#include <vector>
#include <stdexcept>

template <typename T>
T shift(std::vector<T>& vec) {
    if (vec.empty()) {
        std::cerr << "[EmptyVectorShiftError]: Cannot shift an empty vector";
        exit(1);
    }

    T first = vec.front();
    vec.erase(vec.begin());
    return first;
}
