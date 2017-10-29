#pragma once

#include <cinttypes>

namespace meta {

template<class T, T... V>
struct value_list {

    constexpr static bool contains(T v) {
        bool sum = false;
        auto x = {((sum = sum || (v == V)), 0)...};
        (void)x;
        return sum;
    }
};

template<size_t... V>
using index_list = value_list<size_t, V...>;

} // namespace meta
