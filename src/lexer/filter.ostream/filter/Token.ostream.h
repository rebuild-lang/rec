#pragma once
#include "scanner/Token.h"

#include "scanner/Token.ostream.h"

namespace filter {

inline std::ostream& operator<<(std::ostream& out, const IdentifierLiteralValue& ident) {
    return out << (ident.leftSeparated ? "_" : "|") << "id" << (ident.rightSeparated ? "_" : "|");
}

inline std::ostream& operator<<(std::ostream& out, const IdentifierLiteral& ident) {
    return out << "<id: " << (ident.value.leftSeparated ? "_" : "|") // << ident.range
               << (ident.value.rightSeparated ? "_" : "|") << '>';
}

inline std::ostream& operator<<(std::ostream& out, const BlockStartIndentation&) { return out << "<blockStart>"; }
inline std::ostream& operator<<(std::ostream& out, const BlockEndIndentation&) { return out << "<blockEnd>"; }

inline std::ostream& operator<<(std::ostream& out, const Token& t) {
    return t.visit([&](const auto& v) -> decltype(auto) { return out << v; });
}

} // namespace filter
