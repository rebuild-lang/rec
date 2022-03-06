#pragma once
#include "CodePoint.h"

#include "Counter.ostream.h"
#include "meta/Optional.ostream.h"

#include <array>
#include <string_view>

namespace strings {

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, Decimal d) -> ::std::basic_ostream<Cs...>& {
    return out << "digit:" << static_cast<int>(d.v);
}

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, CodePoint cp) -> ::std::basic_ostream<Cs...>& {
    return out << std::hex << std::showbase << cp.v << std::dec << std::noshowbase;
}

} // namespace strings
