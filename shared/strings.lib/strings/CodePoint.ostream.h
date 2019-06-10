#pragma once
#include "CodePoint.h"

#include "Counter.ostream.h"
#include "meta/Optional.ostream.h"

#include <array>
#include <string_view>

namespace strings {

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, Decimal d) -> decltype(out) {
    return out << "digit:" << static_cast<int>(d.v);
}

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, CodePoint cp)
    -> decltype(out << std::declval<std::basic_string_view<Char>>()) {

    return out << std::hex << std::showbase << cp.v << std::dec << std::noshowbase;
}

} // namespace strings
