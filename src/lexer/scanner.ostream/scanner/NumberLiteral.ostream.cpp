#include "NumberLiteral.ostream.h"

#include "strings/Rope.ostream.h"

namespace scanner {

auto operator<<(std::ostream& out, const NumberLiteralValue& lit) -> std::ostream& {
    out << "r=" << lit.radix << " v=[" << lit.integerPart;
    if (!lit.fractionalPart.isEmpty()) out << '.' << lit.fractionalPart;
    if (!lit.exponentPart.isEmpty()) out << 'e' << (lit.exponentSign == Sign::positive ? '+' : '-') << lit.exponentPart;
    return out << "]";
}

} // namespace scanner
