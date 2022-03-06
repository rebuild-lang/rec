#pragma once
#include "View.h"

#include "CodePoint.ostream.h"
#include "String.ostream.h"
#include "join.ostream.h"
#include "meta/Optional.ostream.h"

#include <iomanip>
#include <string_view>

namespace strings {

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, const View& v) -> decltype(auto) {
    if (std::ios_base::hex == (out.flags() & std::ios_base::basefield)) {
        return out << std::setfill('0')
                   << joinEachApply(v, ' ', [&](auto c) { out << std::setw(2) << static_cast<int>(c); });
    }
    return out << std::string_view{v.begin(), v.byteCount().v};
}

template<typename... Cs>
auto operator<<(::std::basic_ostream<Cs...>& out, const CompareView& v) -> decltype(auto) {
    return out << static_cast<const View&>(v);
}

} // namespace strings
