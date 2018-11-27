#pragma once

#include "DecodedPosition.h"

#include "Position.ostream.h"

#include <meta/Variant.ostream.h>
#include <strings/CodePoint.ostream.h>
#include <strings/View.ostream.h>
#include <strings/join.h>

namespace meta {

constexpr auto nameOf(Type<text::CodePointPosition>) { return "CP"; }
constexpr auto nameOf(Type<text::NewlinePosition>) { return "Newline"; }
constexpr auto nameOf(Type<text::DecodedErrorPosition>) { return "Error"; }

} // namespace meta

namespace text {

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, CodePointPosition cpp) -> decltype(out) {
    return out << ": " << std::hex << cpp.input //
               << " = cp: " << cpp.codePoint << cpp.position;
}

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, NewlinePosition nlp) -> decltype(out) {
    return out << ": " << std::hex << nlp.input //
               << nlp.position;
}

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, DecodedErrorPosition dep) -> decltype(out) {
    return out << ": " << std::hex << dep.input //
               << dep.position;
}

} // namespace text
