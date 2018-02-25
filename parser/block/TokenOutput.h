#pragma once
#include "parser/block/Token.h"

#include "parser/filter/TokenOutput.h"

namespace parser::block {

auto operator<<(std::ostream &out, const Token &v) -> std::ostream &;

inline auto operator<<(std::ostream &out, const TokenLine &l) -> std::ostream & {
    for (const auto &tok : l) {
        out << tok;
    }
    return out;
}

inline auto operator<<(std::ostream &out, const BlockLiteral &b) -> std::ostream & {
    for (const auto &line : b.lines) {
        out << "  line: " << line << '\n';
    }
    return out;
}

inline auto operator<<(std::ostream &out, const TokenVariant &v) -> std::ostream & {
    v.visit(
        [&](const BlockLiteral &block) { out << "{block: " << block; },
        [&](const ColonSeparator &) { out << "<:>"; },
        [&](const CommaSeparator &) { out << "<,>"; },
        [&](const SquareBracketOpen &) { out << "<[>"; },
        [&](const SquareBracketClose &) { out << "<]>"; },
        [&](const BracketOpen &) { out << "<(>"; },
        [&](const BracketClose &) { out << "<)>"; },
        [&](const StringLiteral &) { out << "<\"\">"; },
        [&](const NumberLiteral &num) { out << "<num: " << num << ">"; },
        [&](const IdentifierLiteral &ident) { out << "<id: " << ident << ">"; },
        [&](const OperatorLiteral &op) { out << "<op: " << op << ">"; });
    return out;
}

inline auto operator<<(std::ostream &out, const Token &t) -> std::ostream & {
    out << t.range << t.data;
    return out;
}

} // namespace parser::block
