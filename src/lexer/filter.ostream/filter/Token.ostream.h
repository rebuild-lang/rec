#pragma once
#include "filter/Token.h"

#include "scanner/Token.ostream.h"

#include "strings/join.ostream.h"

namespace filter {

inline auto operator<<(std::ostream& out, const BlockStartColon&) -> std::ostream& { return out << "<begin:>"; }
inline auto operator<<(std::ostream& out, const BlockEndIdentifier&) -> std::ostream& { return out << "<end>"; }
inline auto operator<<(std::ostream& out, const UnexpectedColon&) -> std::ostream& { return out << "<X:>"; }

inline std::ostream& operator<<(std::ostream& out, const Token& t) {
    return t.visit([&](const auto& v) -> decltype(auto) { return out << v; });
}
inline std::ostream& operator<<(std::ostream& out, const Insignificant& t) {
    return t.visit([&](const auto& v) -> decltype(auto) { return out << v; });
}

inline std::ostream& operator<<(std::ostream& out, const TokenLine& l) {
    l.forEach([&](const auto& t) { out << t; });
    return out;
}

} // namespace filter
