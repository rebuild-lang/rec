#pragma once
#include "Rope.h"

#include "CodePoint.ostream.h"
#include "String.ostream.h"
#include "View.ostream.h"

#include "meta/Variant.ostream.h"

namespace strings {

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, const Rope& r) -> decltype(auto) {

    return out << to_string(r);
}

} // namespace strings
