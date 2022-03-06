#pragma once
#include "Counter.h"

#include <ostream>

namespace strings {

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, Counter c) -> ::std::basic_ostream<Cs...>& {
    return out << "#" << c.v;
}

} // namespace strings
