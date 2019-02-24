#pragma once
#include "scanner/Token.h"

namespace filter {

namespace details {

using scanner::details::TagToken;
using scanner::details::ValueToken;

} // namespace details

using text::View;

using scanner::NewLineIndentation;

using BlockStartIndentation = details::TagToken<struct BlockStartIndentationTag>;
using BlockEndIndentation = details::TagToken<struct BlockEndIndentationTag>;
using scanner::BracketClose;
using scanner::BracketOpen;
using scanner::ColonSeparator;
using scanner::CommaSeparator;
using scanner::NumberLiteral;
using scanner::SemicolonSeparator;
using scanner::SquareBracketClose;
using scanner::SquareBracketOpen;
using scanner::StringLiteral;
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

} // namespace filter
