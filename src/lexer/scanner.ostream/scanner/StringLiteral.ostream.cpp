#include "StringLiteral.ostream.h"

#include "strings/Rope.ostream.h"

namespace scanner {

auto operator<<(std::ostream& out, const StringLiteralValue& lit) -> std::ostream& {
    out << '"';
    out << lit.text;
    return out << '"';
}

} // namespace scanner
