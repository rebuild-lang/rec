#pragma once
#include "scanner/Token.h"

#include "scanner/NumberLiteralOutput.h"

namespace parser::filter {

using TextRange = scanner::TextRange;
using View = scanner::View;

using NewLineIndentation = scanner::NewLineIndentation;
struct BlockStartIndentation {
    bool operator==(const BlockStartIndentation &) const { return true; }
    bool operator!=(const BlockStartIndentation &o) const { return !(*this == o); }
};
struct BlockEndIndentation {
    bool operator==(const BlockEndIndentation &) const { return true; }
    bool operator!=(const BlockEndIndentation &o) const { return !(*this == o); }
};
using ColonSeparator = scanner::ColonSeparator;
using CommaSeparator = scanner::CommaSeparator;
using SemicolonSeparator = scanner::SemicolonSeparator;
using SquareBracketOpen = scanner::SquareBracketOpen;
using SquareBracketClose = scanner::SquareBracketClose;
using BracketOpen = scanner::BracketOpen;
using BracketClose = scanner::BracketClose;
using StringLiteral = scanner::StringLiteral;
using NumberLiteral = scanner::NumberLiteral;
struct IdentifierLiteral {
    bool leftSeparated = false;
    bool rightSeparated = false;

    bool operator==(const IdentifierLiteral &o) const {
        return leftSeparated == o.leftSeparated && rightSeparated == o.rightSeparated;
    }
    bool operator!=(const IdentifierLiteral &o) const { return !(*this == o); }
};
struct OperatorLiteral : IdentifierLiteral {};

using TokenVariant = meta::Variant<
    NewLineIndentation,
    BlockStartIndentation,
    BlockEndIndentation,
    ColonSeparator,
    CommaSeparator,
    SemicolonSeparator,
    SquareBracketOpen,
    SquareBracketClose,
    BracketOpen,
    BracketClose,
    StringLiteral,
    NumberLiteral,
    IdentifierLiteral,
    OperatorLiteral>;

struct Token {
    TextRange range;
    TokenVariant data;

    template<class... Ts>
    bool oneOf() const {
        return data.holds<Ts...>();
    }
};

} // namespace parser::filter
