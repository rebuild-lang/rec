#pragma once
#include "DecodedPosition.h"

#include "Position.ostream.h"

#include <meta/Variant.ostream.h>
#include <strings/CodePoint.ostream.h>
#include <strings/View.ostream.h>

namespace meta {

constexpr auto nameOf(Type<text::CodePointPosition>) { return "CP"; }
constexpr auto nameOf(Type<text::NewlinePosition>) { return "Newline"; }
constexpr auto nameOf(Type<text::DecodedErrorPosition>) { return "Error"; }

} // namespace meta

namespace text {

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, CodePointPosition cpp) -> decltype(out) {
    return out << ": " << std::hex << cpp.input //
               << " = cp: " << cpp.codePoint << cpp.position;
}

template<typename... Cs, class... Tags>
auto operator<<(::std::basic_ostream<Cs...>& out, InputPosition<Tags...> ip) -> decltype(out) {
    return out << ": " << std::hex << ip.input << ip.position;
}

} // namespace text
