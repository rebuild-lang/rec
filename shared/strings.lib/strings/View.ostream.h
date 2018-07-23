#pragma once
#include "View.h"

#include "CodePoint.ostream.h"
#include "String.ostream.h"
#include "meta/Optional.ostream.h"

#include <string_view>

namespace strings {

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, const View& v) -> decltype(auto) {

    using StrView = std::basic_string_view<Char>;
    return out << StrView{reinterpret_cast<typename StrView::const_pointer>(v.begin()), v.byteCount().v};
}

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, const CompareView& v) -> decltype(auto) {

    using StrView = std::basic_string_view<Char>;
    return out << StrView{reinterpret_cast<typename StrView::const_pointer>(v.begin()), v.byteCount().v};
}

} // namespace strings
