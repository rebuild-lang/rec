#pragma once
#include "strings/rope.h"

#include <ostream>

namespace scanner {

using rope_t = strings::rope;
using view_t = strings::utf8_view;

enum class sign_t : bool { positive = false, negative = true };
constexpr auto to_string(sign_t sign) -> view_t {
    switch (sign) {
    case sign_t::positive: return view_t{"+"};
    case sign_t::negative: return view_t{"-"};
    }
    return view_t{"[corrupted-sign]"};
}
inline auto operator<<(std::ostream &out, sign_t sign) -> std::ostream & { return out << to_string(sign); }

enum class radix_t : int {
    invalid = 0,
    binary = 1,
    octal = 8,
    decimal = 10,
    hex = 16,
};
constexpr auto to_string(radix_t radix) -> view_t {
    switch (radix) {
    case radix_t::invalid: return view_t{"invalid"};
    case radix_t::binary: return view_t{"binary"};
    case radix_t::octal: return view_t{"octal"};
    case radix_t::decimal: return view_t{"decimal"};
    case radix_t::hex: return view_t{"hex"};
    }
    return view_t{"[corrupted-radix]"};
}
inline auto operator<<(std::ostream &out, radix_t radix) -> std::ostream & { return out << to_string(radix); }

struct number_literal_t {
    radix_t radix = radix_t::invalid;
    rope_t integer_part;
    rope_t fractional_part;
    sign_t exponent_sign = sign_t::positive;
    rope_t exponent_part;

    explicit operator bool() const { return radix != radix_t::invalid; }

    constexpr bool operator==(const number_literal_t &o) const noexcept {
        if (radix == radix_t::invalid || o.radix == radix_t::invalid) // fast optional invalid
            return radix == o.radix;
        return radix == o.radix //
               && integer_part == o.integer_part //
               && fractional_part == o.fractional_part //
               && (exponent_part.is_empty() ? true : exponent_sign == o.exponent_sign) //
               && exponent_part == o.exponent_part;
    }
    constexpr bool operator!=(const number_literal_t &o) const noexcept { return !(*this == o); }
};
inline auto operator<<(std::ostream &out, const number_literal_t &lit) -> std::ostream & {
    out << "r=" << lit.radix << " v=[" << lit.integer_part;
    if (!lit.fractional_part.is_empty()) out << '.' << lit.fractional_part;
    if (!lit.exponent_part.is_empty())
        out << 'e' << (lit.exponent_sign == sign_t::positive ? '+' : '-') << lit.exponent_part;
    return out << "]";
}

} // namespace scanner
