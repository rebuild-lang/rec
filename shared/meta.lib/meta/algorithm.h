#pragma once
#include "Optional.h"

#include <algorithm>
#include <numeric>

/// convenience algorithms that work on containers
namespace meta {

template<class C, class T, class F>
auto accumulate(const C& c, T&& init, F&& f) {
    using std::begin;
    using std::end;
    return std::accumulate(begin(c), end(c), std::forward<T>(init), std::forward<F>(f));
}

template<class C, class T>
auto find(const C& c, const T& v) {
    using std::begin;
    using std::end;
    return std::find(begin(c), end(c), v);
}

template<class C, class T>
auto findNot(const C& c, const T& v) {
    using std::begin;
    using std::end;
    return std::find_if(begin(c), end(c), [&](const auto& n) { return n != v; });
}

template<class C, class F>
auto findIf(const C& c, F&& f) -> decltype(c.begin()) {
    using std::begin;
    using std::end;
    return std::find_if(begin(c), end(c), std::forward<F>(f));
}

template<class C, class F>
auto findIfOpt(const C& c, F&& f) -> Optional<decltype(c.front())> {
    auto it = findIf(c, std::forward<F>(f));
    if (it != end(c)) return *it;
    return {};
}

template<class C, class F>
auto stableSort(C& c, F&& f) {
    using std::begin;
    using std::end;
    std::stable_sort(begin(c), end(c), std::forward<F>(f));
}

template<class C, class F>
auto stablePartition(C& c, F&& f) -> decltype(c.begin()) {
    using std::begin;
    using std::end;
    return std::stable_partition(begin(c), end(c), std::forward<F>(f));
}

template<class O, class C, class F>
auto collect(O& o, const C& c, F&& f) {
    using std::begin;
    using std::end;
    std::transform(begin(c), end(c), std::back_inserter(o), std::forward<F>(f));
    return o;
}

template<class O, class T>
auto append(O& o, const T& t) {
    using std::begin;
    using std::end;
    return o.insert(o.end(), begin(t), end(t));
}

template<class T, class I>
constexpr auto equals(T t, I i) {
    for (const auto& v : t) {
        if (!(v == *i)) return false;
        i++;
    }
    return true;
}

} // namespace meta
