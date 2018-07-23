#pragma once

#include "parser/filter/Token.h"

namespace parser::block {

namespace details {

using parser::filter::details::ValueToken;
}

using TextRange = text::TextRange;

struct Token;
using TokenLine = std::vector<Token>;
struct BlockLiteralValue {
    using This = BlockLiteralValue;
    std::vector<TokenLine> lines{};

    bool operator!=(const This& o) const { return lines != o.lines; }
    bool operator==(const This& o) const { return lines == o.lines; }
};
using BlockLiteral = details::ValueToken<BlockLiteralValue>;
using ColonSeparator = filter::ColonSeparator;
using CommaSeparator = filter::CommaSeparator;
using SquareBracketOpen = filter::SquareBracketOpen;
using SquareBracketClose = filter::SquareBracketClose;
using BracketOpen = filter::BracketOpen;
using BracketClose = filter::BracketClose;
using StringLiteral = filter::StringLiteral;
using NumberLiteral = filter::NumberLiteral;
using IdentifierLiteral = filter::IdentifierLiteral;
using OperatorLiteral = filter::OperatorLiteral;

using TokenVariant = meta::Variant<
    BlockLiteral,
    ColonSeparator,
    CommaSeparator,
    SquareBracketOpen,
    SquareBracketClose,
    BracketOpen,
    BracketClose,
    StringLiteral,
    NumberLiteral,
    IdentifierLiteral,
    OperatorLiteral>;

struct Token : TokenVariant {
    META_VARIANT_CONSTRUCT(Token, TokenVariant)
};

} // namespace parser::block
