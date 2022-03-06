#pragma once
#include <stddef.h> // size_t

namespace meta {

template<class T, T... V>
struct ValueList {

    constexpr static bool contains(T v) {
        bool sum = false;
        auto x = {((sum = sum || (v == V)), 0)...};
        (void)x;
        return sum;
    }
};

template<size_t... V>
using IndexList = ValueList<size_t, V...>;

} // namespace meta
