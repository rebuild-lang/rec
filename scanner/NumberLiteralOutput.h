#pragma once
#include "NumberLiteral.h"

#include "strings/Output.h"

namespace scanner {

inline auto operator<<(std::ostream& out, Sign sign) -> std::ostream& { return out << to_string(sign); }

inline auto operator<<(std::ostream& out, Radix radix) -> std::ostream& { return out << to_string(radix); }

auto operator<<(std::ostream& out, const NumberLiteralValue& lit) -> std::ostream&;

} // namespace scanner
