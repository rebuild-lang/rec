#include "NumberLiteralValue.ostream.h"

#include <strings/Rope.ostream.h>
#include <strings/join.ostream.h>
#include <text/DecodedPosition.ostream.h>

namespace scanner {

auto operator<<(std::ostream& out, const NumberLiteralValue& lit) -> std::ostream& {
    out << "r=" << lit.radix << " v=[" << lit.integerPart;
    if (!lit.fractionalPart.isEmpty()) out << '.' << lit.fractionalPart;
    if (!lit.exponentPart.isEmpty()) out << 'e' << (lit.exponentSign == Sign::positive ? '+' : '-') << lit.exponentPart;
    out << "]";
    if (!lit.errors.empty()) out << " errors: " << strings::joinEach(lit.errors, ", ");
    return out;
}

} // namespace scanner
