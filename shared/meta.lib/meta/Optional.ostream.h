#pragma once
#include "Optional.h"

#include <ostream>

template<typename Char, typename CharTraits, class T>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, const meta::Optional<T>& opt)
    -> decltype(out << std::declval<meta::Optional<T>>().value()) {

    if (!opt) return out << "<Empty>";
    return out << opt.value();
}
