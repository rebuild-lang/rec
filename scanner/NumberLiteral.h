#pragma once
#include "strings/Rope.h"

namespace scanner {

using Rope = strings::Rope;
using View = strings::View;

enum class Sign : bool { positive = false, negative = true };
constexpr auto to_string(Sign sign) -> View {
    switch (sign) {
    case Sign::positive: return View{"+"};
    case Sign::negative: return View{"-"};
    }
    return View{"[corrupted-sign]"};
}

enum class Radix : int {
    invalid = 0,
    binary = 1,
    octal = 8,
    decimal = 10,
    hex = 16,
};
constexpr auto to_string(Radix radix) -> View {
    switch (radix) {
    case Radix::invalid: return View{"invalid"};
    case Radix::binary: return View{"binary"};
    case Radix::octal: return View{"octal"};
    case Radix::decimal: return View{"decimal"};
    case Radix::hex: return View{"hex"};
    }
    return View{"[corrupted-radix]"};
}

struct NumberLiteralValue {
    using This = NumberLiteralValue;
    Radix radix{Radix::invalid};
    Rope integerPart{};
    Rope fractionalPart{};
    Sign exponentSign{Sign::positive};
    Rope exponentPart{};

    explicit operator bool() const { return radix != Radix::invalid; }

    constexpr bool operator==(const This& o) const noexcept {
        if (radix == Radix::invalid || o.radix == Radix::invalid) return radix == o.radix; // fast optional invalid
        return radix == o.radix //
            && integerPart == o.integerPart //
            && fractionalPart == o.fractionalPart //
            && (exponentPart.isEmpty() ? true : exponentSign == o.exponentSign) //
            && exponentPart == o.exponentPart;
    }
    constexpr bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

} // namespace scanner
