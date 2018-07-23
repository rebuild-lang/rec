#pragma once
#include "scanner/Token.h"

#include "scanner/NumberLiteralOutput.h"

namespace parser::filter {

namespace details {

using scanner::details::TagToken;
using scanner::details::ValueToken;

} // namespace details

using TextRange = text::TextRange;
using View = text::View;

using NewLineIndentation = scanner::NewLineIndentation;

using BlockStartIndentation = details::TagToken<struct BlockStartIndentationTag>;
using BlockEndIndentation = details::TagToken<struct BlockEndIndentationTag>;
using ColonSeparator = scanner::ColonSeparator;
using CommaSeparator = scanner::CommaSeparator;
using SemicolonSeparator = scanner::SemicolonSeparator;
using SquareBracketOpen = scanner::SquareBracketOpen;
using SquareBracketClose = scanner::SquareBracketClose;
using BracketOpen = scanner::BracketOpen;
using BracketClose = scanner::BracketClose;
using StringLiteral = scanner::StringLiteral;
using NumberLiteral = scanner::NumberLiteral;
struct IdentifierLiteralValue {
    using This = IdentifierLiteralValue;
    bool leftSeparated{false};
    bool rightSeparated{false};

    bool operator==(const This& o) const {
        return leftSeparated == o.leftSeparated && rightSeparated == o.rightSeparated;
    }
    bool operator!=(const This& o) const { return !(*this == o); }
};
using IdentifierLiteral = details::ValueToken<IdentifierLiteralValue>;
struct OperatorLiteral : IdentifierLiteral {};

using Token = meta::Variant<
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

} // namespace parser::filter
