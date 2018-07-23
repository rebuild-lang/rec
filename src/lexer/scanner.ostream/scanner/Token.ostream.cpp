#include "Token.ostream.h"

namespace scanner {

auto operator<<(std::ostream& out, const Token& v) -> std::ostream& {
    v.visit(
        [&](const WhiteSpaceSeparator&) { out << "<space>"; },
        [&](const NewLineIndentation&) { out << "<\\n>"; },
        [&](const CommentLiteral& c) { out << "<##: " << c.range << ">"; },
        [&](const ColonSeparator&) { out << "<:>"; },
        [&](const CommaSeparator&) { out << "<,>"; },
        [&](const SemicolonSeparator&) { out << "<;>"; },
        [&](const SquareBracketOpen&) { out << "<[>"; },
        [&](const SquareBracketClose&) { out << "<]>"; },
        [&](const BracketOpen&) { out << "<(>"; },
        [&](const BracketClose&) { out << "<)>"; },
        [&](const StringLiteral& str) { out << "<str: " << str.value << ">"; },
        [&](const NumberLiteral& num) { out << "<num: " << num.value << ">"; },
        [&](const IdentifierLiteral& id) { out << "<id: " << id.range << ">"; },
        [&](const OperatorLiteral& op) { out << "<op: " << op.range << ">"; },
        [&](const InvalidEncoding& e) { out << "<EncErr: " << e.range << ">"; },
        [&](const UnexpectedCharacter& u) { out << "<Unexp: " << u.range << ">"; });
    return out;
}

} // namespace scanner
