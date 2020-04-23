#pragma once
#include <scanner/IdentifierLiteralValue.h>

#include <strings/View.ostream.h>

namespace scanner {

inline auto operator<<(std::ostream& out, IdentifierLiteralType type) -> std::ostream& {
    return out << to_string(type);
}

} // namespace scanner
