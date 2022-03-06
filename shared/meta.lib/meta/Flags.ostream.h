#pragma once
#include "Flags.h"

#include <ostream>

template<typename... Cs, class T>
auto operator<<(::std::basic_ostream<Cs...>& out, meta::Flags<T> f) //
    -> decltype(out << std::declval<T>()) {

    using flags_type = meta::Flags<T>;
    if (f == flags_type{}) return out << "<None>";
    f.each_set([&, first = true](T t) mutable {
        if (!first)
            out << " | ";
        else
            first = false;
        out << t;
    });
    return out;
}
