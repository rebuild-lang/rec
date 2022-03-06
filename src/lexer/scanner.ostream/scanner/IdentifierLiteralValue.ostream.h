#pragma once
#include <scanner/IdentifierLiteralValue.h>

#include <strings/View.ostream.h>

namespace scanner {

inline auto operator<<(std::ostream& out, IdentifierLiteralType literalType) -> std::ostream& {
    return out << to_string(literalType);
}

} // namespace scanner
