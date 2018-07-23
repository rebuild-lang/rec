#pragma once

#include <string>

namespace strings {

template<class Out, class Container, class Str>
auto join(Out& out, const Container& container, const Str& delimiter) {
    auto i = 0u;
    auto l = container.size();
    for (const auto& e : container) {
        out << e;
        if (++i != l) out << delimiter;
    }
}

} // namespace strings
