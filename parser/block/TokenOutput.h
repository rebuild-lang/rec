#pragma once
#include "parser/block/Token.h"

#include "parser/filter/TokenOutput.h"

namespace parser::block {

auto operator<<(std::ostream& out, const Token& v) -> std::ostream&;

inline auto operator<<(std::ostream& out, const TokenLine& l) -> std::ostream& {
    for (const auto& tok : l) {
        out << tok;
    }
    return out;
}

inline auto operator<<(std::ostream& out, const BlockLiteralValue& b) -> std::ostream& {
    for (const auto& line : b.lines) {
        out << "  line: " << line << '\n';
    }
    return out;
}

inline auto operator<<(std::ostream& out, const Token& v) -> std::ostream& {
    v.visit(
        [&](const BlockLiteral& block) { out << "{block: " << block.value; },
        [&](const ColonSeparator&) { out << "<:>"; },
        [&](const CommaSeparator&) { out << "<,>"; },
        [&](const SquareBracketOpen&) { out << "<[>"; },
        [&](const SquareBracketClose&) { out << "<]>"; },
        [&](const BracketOpen&) { out << "<(>"; },
        [&](const BracketClose&) { out << "<)>"; },
        [&](const StringLiteral& str) { out << "<str: " << str.value << ">"; },
        [&](const NumberLiteral& num) { out << "<num: " << num.value << ">"; },
        [&](const IdentifierLiteral& ident) { out << "<id: " << ident << ">"; },
        [&](const OperatorLiteral& op) { out << "<op: " << op << ">"; });
    return out;
}

} // namespace parser::block
