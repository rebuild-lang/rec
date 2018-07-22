#pragma once
#include "CodePoint.h"

#include "Counter.ostream.h"
#include "meta/Optional.ostream.h"

#include <array>
#include <string_view>

namespace strings {

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, Decimal d) -> decltype(out << (int)d.v) {
    return out << "digit:" << (int)d.v;
}

template<typename Char, typename CharTraits>
auto operator<<(::std::basic_ostream<Char, CharTraits>& out, CodePoint cp)
    -> decltype(out << std::declval<std::basic_string_view<Char>>()) {

    using StrView = std::basic_string_view<Char>;
    class CodeStr {
        std::array<uint8_t, 8> d{};
        size_t s{};

    public:
        void push_back(uint8_t c) { d[s++] = c; }
        auto view() { return StrView{reinterpret_cast<typename StrView::const_pointer>(d.data()), s}; }
    } codeStr{};

    cp.utf8_encode(codeStr);
    return out << codeStr.view();
}

} // namespace strings
