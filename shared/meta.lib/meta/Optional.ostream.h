#pragma once
#include "Optional.h"

#include <ostream>

template<typename Char, typename CharTraits, class T>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, const meta::Optional<T>& opt)
    -> decltype(out << std::declval<T>()) {

    if (!opt) return out << "<Empty>";
    return out << opt.value();
}
