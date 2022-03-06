#pragma once

#include "CodePoint.ostream.h"
#include "Decoded.h"
#include "View.ostream.h"
#include "join.h"

#include <meta/Variant.ostream.h>
#include <ostream>

namespace meta {

constexpr auto nameOf(Type<strings::DecodedError>) { return "Error"; }
constexpr auto nameOf(Type<strings::DecodedCodePoint>) { return "CP"; }

} // namespace meta

namespace strings {

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, DecodedError de) -> decltype(out) {
    return out << ": " << std::hex << de.input << std::dec;
}

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, DecodedCodePoint dcp) -> decltype(out) {
    return out << ": " << std::hex << dcp.input //
               << " = cp: " << dcp.cp;
}

} // namespace strings
