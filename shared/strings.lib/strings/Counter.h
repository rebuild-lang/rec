#pragma once
#include <cinttypes>

namespace strings {

/// naively strong typed counter
struct Counter {
    using This = Counter;
    uint32_t v{};

    constexpr bool operator==(This o) const noexcept { return v == o.v; }
    constexpr bool operator!=(This o) const noexcept { return v != o.v; }

    constexpr auto operator+(This c) const noexcept { return This{v + c.v}; }
};

} // namespace strings
