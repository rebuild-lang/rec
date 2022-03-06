#pragma once
#include "Rope.h"

#include "CodePoint.ostream.h"
#include "String.ostream.h"
#include "View.ostream.h"

#include "meta/Variant.ostream.h"

namespace strings {

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, const Rope& r) -> decltype(out) {
    return out << to_string(r);
}

} // namespace strings
