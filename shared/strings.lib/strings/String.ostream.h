#pragma once
#include "String.h"

#include "Counter.ostream.h"
#include "meta/Optional.ostream.h"

#include <string_view>

namespace strings {

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, const String& s) -> decltype(out) {
    using StrView = std::basic_string_view<Cs...>;
    auto ptr = reinterpret_cast<typename StrView::const_pointer>(s.begin());
    return out << StrView{ptr, s.byteCount().v};
}

} // namespace strings
