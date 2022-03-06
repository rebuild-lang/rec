#pragma once
#include "Optional.h"

#include <ostream>

namespace meta {

template<class... Cs, class T>
auto operator<<(::std::basic_ostream<Cs...>& out, const Optional<T>& opt)
    -> decltype(out << std::declval<Optional<T>>().value()) {

    if (!opt) return out << "<Empty>";
    return out << opt.value();
}

} // namespace meta
