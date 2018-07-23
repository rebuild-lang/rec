#pragma once
#include "Counter.h"

#include <ostream>

namespace strings {

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, Counter c) -> decltype(out << c.v) {
    return out << "#" << c.v;
}

} // namespace strings
