#pragma once
#include <strings/Rope.h>
#include <text/DecodedPosition.h>

#include <vector>

namespace scanner {

using strings::Rope;
using strings::View;
using text::DecodedErrorPosition;

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

using NumberMissingExponent = text::InputPosition<struct NumberMissingExponentTag>;
using NumberMissingValue = text::InputPosition<struct NumberMissingValueTag>;
using NumberMissingBoundary = text::InputPosition<struct NumberMissingBoundaryTag>;

using NumberLiteralError = meta::Variant< //
    DecodedErrorPosition,
    NumberMissingExponent, // "1e", "1e-", "1e+" (after "e" a value is expected)
    NumberMissingValue, // "0x", "0x." (after radix sign a value is expected)
    NumberMissingBoundary // "0z", "0_", … (suffixes are not handled but reserved!)
    >;
using NumberLiteralErrors = std::vector<NumberLiteralError>;

struct NumberLiteralValue {
    using This = NumberLiteralValue;
    Radix radix{Radix::invalid};
    Rope integerPart{};
    Rope fractionalPart{};
    Sign exponentSign{Sign::positive};
    Rope exponentPart{};
    NumberLiteralErrors errors{};

    constexpr auto hasErrors() const { return radix == Radix::invalid || !errors.empty(); }

    bool operator==(const This& o) const noexcept {
        if (radix == Radix::invalid || o.radix == Radix::invalid) return radix == o.radix; // fast optional invalid
        return radix == o.radix //
            && integerPart == o.integerPart //
            && fractionalPart == o.fractionalPart //
            && (exponentPart.isEmpty() ? true : exponentSign == o.exponentSign) //
            && exponentPart == o.exponentPart //
            && errors == o.errors;
    }
    bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

} // namespace scanner
