#pragma once

#include "parser/filter/Token.h"

namespace parser::block {

using TextRange = scanner::TextRange;

struct Token;
using TokenLine = std::vector<Token>;
struct BlockLiteral {
    std::vector<TokenLine> lines{};

    bool operator==(const BlockLiteral& o) const { return lines == o.lines; }
    bool operator!=(const BlockLiteral& o) const { return lines != o.lines; }
};
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

struct Token {
    TextRange range{};
    TokenVariant data{};

    template<class... Ts>
    bool oneOf() const {
        return data.holds<Ts...>();
    }

    bool operator==(const Token& o) const { return range == o.range && data == o.data; }
    bool operator!=(const Token& o) const { return !(*this == o); }
};

} // namespace parser::block
