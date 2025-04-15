#pragma once
#include <vector>
#include <stdexcept>

template <typename T>
T shift(std::vector<T>& vec) {
    if (vec.empty()) {
        throw std::out_of_range("Kan ikke shift() fra tom vektor.");
    }

    T first = vec.front();
    vec.erase(vec.begin());
    return first;
}
