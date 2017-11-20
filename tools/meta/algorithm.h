#pragma once
#include "optional.h"

#include <algorithm>
#include <numeric>

/// convenience algorithms that work on containers
namespace meta {

template<class C, class T, class F>
auto accumulate(const C &c, T &&init, F &&f) {
    using std::begin;
    using std::end;
    return std::accumulate(begin(c), end(c), std::forward<T>(init), std::forward<F>(f));
}

template<class C, class T>
auto find(const C &c, const T &v) {
    using std::begin;
    using std::end;
    return std::find(begin(c), end(c), v);
}

template<class C, class F>
auto find_if(const C &c, F &&f) -> optional<decltype(c.front())> {
    using std::begin;
    using std::end;
    auto it = std::find_if(begin(c), end(c), std::forward<F>(f));
    if (it != end(c)) return *it;
    return {};
}

template<class O, class C, class F>
auto collect(O &o, const C &c, F &&f) {
    using std::begin;
    using std::end;
    std::transform(begin(c), end(c), std::back_inserter(o), std::forward<F>(f));
    return o;
}

template<class O, class T>
auto append(O &o, const T &t) {
    using std::begin;
    using std::end;
    return o.insert(o.end(), begin(t), end(t));
}

template<class T, class I>
constexpr auto equals(const T &t, I i) {
    for (const auto &v : t) {
        if (!(v == *i)) return false;
        i++;
    }
    return true;
}

} // namespace meta
