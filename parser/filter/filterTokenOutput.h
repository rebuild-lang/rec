#pragma once
#include "parser/filter/filterToken.h"

#include "scanner/TokenOutput.h"

namespace parser::filter {

inline std::ostream &operator<<(std::ostream &out, const IdentifierLiteral &ident) {
    return out << (ident.leftSeparated ? "_" : "|") << "T" << (ident.rightSeparated ? "_" : "|");
}

inline std::ostream &operator<<(std::ostream &out, const TokenVariant &v) {
    v.visit(
        [&](const NewLineIndentation &) { out << "<\\n>"; },
        [&](const BlockStartIndentation &) { out << "<{>"; },
        [&](const BlockEndIndentation &) { out << "<}>"; },
        [&](const ColonSeparator &) { out << "<:>"; },
        [&](const CommaSeparator &) { out << "<,>"; },
        [&](const SemicolonSeparator &) { out << "<;>"; },
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

inline std::ostream &operator<<(std::ostream &out, const Token &t) {
    out << t.range << t.data;
    return out;
}

} // namespace parser::filter
