#pragma once

#include <string>

namespace strings {

template<class Out, class Container, class Delim>
auto join(Out& out, const Container& container, const Delim& delimiter) -> Out& {
    auto i = 0u;
    auto l = container.size();
    for (const auto& e : container) {
        out << e;
        if (++i != l) out << delimiter;
    }
    return out;
}

} // namespace strings
