#pragma once
#include <scanner/NumberLiteralValue.h>

#include <meta/Variant.ostream.h>
#include <strings/View.ostream.h>

namespace meta {

constexpr auto nameOf(Type<scanner::NumberMissingExponent>) { return "MissingExponent"; }
constexpr auto nameOf(Type<scanner::NumberMissingValue>) { return "MissingValue"; }
constexpr auto nameOf(Type<scanner::NumberMissingBoundary>) { return "MissingBoundary"; }

} // namespace meta

namespace scanner {

inline auto operator<<(std::ostream& out, Sign sign) -> std::ostream& { return out << to_string(sign); }
inline auto operator<<(std::ostream& out, Radix radix) -> std::ostream& { return out << to_string(radix); }

auto operator<<(std::ostream& out, const NumberLiteralValue& lit) -> std::ostream&;

} // namespace scanner
