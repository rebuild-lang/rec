#pragma once
#include <strings/Rope.h>
#include <text/DecodedPosition.h>

#include <vector>

namespace scanner {

using strings::Rope;
using strings::View;
using text::DecodedErrorPosition;
using DecodedErrorPositions = std::vector<DecodedErrorPosition>;

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

enum class NumberLiteralError {
    None,
    MissingExponent, // "1e", "1e-", "1e+"
    MissingValue, // "0x", "0x."
    MissingBoundary, // "0z", "0_", … (suffixes are not handled but reserved!)
};
constexpr auto to_string(NumberLiteralError error) -> View {
    switch (error) {
    case NumberLiteralError::None: return View{"none"};
    case NumberLiteralError::MissingExponent: return View{"MissingExponent"};
    case NumberLiteralError::MissingValue: return View{"MissingValue"};
    case NumberLiteralError::MissingBoundary: return View{"MissingBoundary"};
    }
    return View{"[corrupted-error]"};
}

struct NumberLiteralValue {
    using This = NumberLiteralValue;
    Radix radix{Radix::invalid};
    Rope integerPart{};
    Rope fractionalPart{};
    Sign exponentSign{Sign::positive};
    Rope exponentPart{};
    NumberLiteralError error{NumberLiteralError::None};
    DecodedErrorPositions decodeErrors{};

    explicit operator bool() const {
        return radix != Radix::invalid && error == NumberLiteralError::None && decodeErrors.empty();
    }

    constexpr bool operator==(const This& o) const noexcept {
        if (radix == Radix::invalid || o.radix == Radix::invalid) return radix == o.radix; // fast optional invalid
        return radix == o.radix //
            && integerPart == o.integerPart //
            && fractionalPart == o.fractionalPart //
            && (exponentPart.isEmpty() ? true : exponentSign == o.exponentSign) //
            && exponentPart == o.exponentPart //
            && error == o.error //
            && decodeErrors == o.decodeErrors;
    }
    constexpr bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

} // namespace scanner
