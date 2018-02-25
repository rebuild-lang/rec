#include "TokenOutput.h"

namespace scanner {

auto operator<<(std::ostream &out, const TokenVariant &v) -> std::ostream & {
    v.visit(
        [&](const WhiteSpaceSeparator &) { out << "<space>"; },
        [&](const NewLineIndentation &) { out << "<\\n>"; },
        [&](const CommentLiteral &) { out << "<##>"; },
        [&](const ColonSeparator &) { out << "<:>"; },
        [&](const CommaSeparator &) { out << "<,>"; },
        [&](const SemicolonSeparator &) { out << "<;>"; },
        [&](const SquareBracketOpen &) { out << "<[>"; },
        [&](const SquareBracketClose &) { out << "<]>"; },
        [&](const BracketOpen &) { out << "<(>"; },
        [&](const BracketClose &) { out << "<)>"; },
        [&](const StringLiteral &str) { out << "<str: " << str << ">"; },
        [&](const NumberLiteral &num) { out << "<num: " << num << ">"; },
        [&](const IdentifierLiteral &id) { out << "<id: " << id << ">"; },
        [&](const OperatorLiteral &op) { out << "<op: " << op << ">"; },
        [&](const InvalidEncoding &) { out << "<E:enc>"; },
        [&](const UnexpectedCharacter &) { out << "<E:exp>"; });
    return out;
}

} // namespace scanner
