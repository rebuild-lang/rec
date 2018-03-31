#include "StringLiteralOutput.h"

namespace scanner {

auto operator<<(std::ostream& out, const StringLiteral& lit) -> std::ostream& {
    out << '"';
    out << lit.text;
    return out << '"';
}

} // namespace scanner
