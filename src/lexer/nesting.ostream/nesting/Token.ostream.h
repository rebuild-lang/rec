#pragma once
#include "nesting/Token.h"

#include "filter/Token.ostream.h"

namespace nesting {

inline static auto indentIndex = std::ios_base::xalloc();

auto operator<<(std::ostream& out, const Token& v) -> std::ostream&;

inline auto operator<<(std::ostream& out, const TokenLine& l) -> std::ostream& {
    for (const auto& tok : l) {
        out << tok;
    }
    return out;
}

inline auto operator<<(std::ostream& out, const BlockLiteralValue& b) -> std::ostream& {
    auto parentIndent = out.iword(indentIndex);
    auto indent = parentIndent + 1;
    out.iword(indentIndex) = indent;

    for (const auto& line : b.lines) {
        for (auto i = 0; i <= indent; i++) out << "  ";
        out << line << '\n';
    }
    out.iword(indentIndex) = parentIndent;
    return out;
}

inline auto operator<<(std::ostream& out, const BlockLiteral& b) -> std::ostream& {
    out << "begin:\n" << b.value;
    auto indent = out.iword(indentIndex);
    for (auto i = 0; i <= indent; i++) out << "  ";
    return out << "end\n";
}

inline auto operator<<(std::ostream& out, const Token& t) -> std::ostream& {
    return t.visit([&](const auto& v) -> decltype(auto) { return out << v; });
}

} // namespace nesting
