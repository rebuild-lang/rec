#pragma once
#include "parser/filter/Token.h"

#include "scanner/TokenOutput.h"

namespace parser::filter {

inline std::ostream& operator<<(std::ostream& out, const IdentifierLiteral& ident) {
    return out << (ident.value.leftSeparated ? "_" : "|") << ident.range << (ident.value.rightSeparated ? "_" : "|");
}

inline std::ostream& operator<<(std::ostream& out, const Token& v) {
    v.visit(
        [&](const NewLineIndentation&) { out << "<\\n>"; },
        [&](const BlockStartIndentation&) { out << "<{>"; },
        [&](const BlockEndIndentation&) { out << "<}>"; },
        [&](const ColonSeparator&) { out << "<:>"; },
        [&](const CommaSeparator&) { out << "<,>"; },
        [&](const SemicolonSeparator&) { out << "<;>"; },
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

} // namespace parser::filter
