#pragma once
#include <variant>

namespace meta {

/// renaming of std::variant
// use is until we find reason to roll our own
template<class... T>
using variant = std::variant<T...>;

using std::visit;

} // namespace meta
