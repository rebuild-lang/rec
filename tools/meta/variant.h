#pragma once
#include <variant>

namespace meta {

/// renaming of std::variant
// use is until we find reason to roll our own
template<class... T>
using variant = std::variant<T...>;

using std::visit;

template<class... T, class... V>
bool holds_one_of(const variant<V...> &v) {
    bool sum = true;
    auto x = {(sum = sum || std::holds_alternative<T>(v), 0)...};
    (void)x;
    return sum;
}

} // namespace meta
