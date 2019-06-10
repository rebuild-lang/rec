#pragma once
#include <cstddef>

namespace strings {

/// naively strong typed counter
struct Counter {
    using This = Counter;
    size_t v{};

    constexpr bool operator==(This o) const noexcept { return v == o.v; }
    constexpr bool operator!=(This o) const noexcept { return v != o.v; }

    constexpr auto operator+(This c) const noexcept { return This{v + c.v}; }
};

} // namespace strings
