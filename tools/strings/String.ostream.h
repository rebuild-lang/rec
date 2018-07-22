#pragma once
#include "String.h"

#include "Counter.ostream.h"
#include "meta/Optional.ostream.h"

#include <string_view>

namespace strings {

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, const String& s) -> decltype(auto) {

    using StrView = std::basic_string_view<Char>;
    return out << StrView{reinterpret_cast<typename StrView::const_pointer>(s.begin()), s.byteCount().v};
}

} // namespace strings
