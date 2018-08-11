#pragma once
#include "NumberLiteral.h"
#include "StringLiteral.h"

#include "text/Range.h"

#include "meta/Optional.h"

namespace scanner {

namespace details {
template<class Tag>
struct TagToken {
    text::Range range{};

    using This = TagToken;
    bool operator==(const This& o) const { return true; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
template<class Value>
struct ValueToken {
    Value value{};
    text::Range range{};

    using This = ValueToken;
    bool operator==(const This& o) const { return value == o.value; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
} // namespace details

using WhiteSpaceSeparator = details::TagToken<struct WhiteSpaceSeparatorTag>;
using NewLineIndentation = details::TagToken<struct NewLineIndentationTag>;
using CommentLiteral = details::TagToken<struct CommentLiteralTag>;
using ColonSeparator = details::TagToken<struct ColonSeparatorTag>;
using CommaSeparator = details::TagToken<struct CommaSeparatorTag>;
using SemicolonSeparator = details::TagToken<struct SemicolonSeparatorTag>;
using SquareBracketOpen = details::TagToken<struct SquareBracketOpenTag>;
using SquareBracketClose = details::TagToken<struct SquareBracketCloseTag>;
using BracketOpen = details::TagToken<struct BracketOpenTag>;
using BracketClose = details::TagToken<struct BracketCloseTag>;
using IdentifierLiteral = details::TagToken<struct IdentifierLiteralTag>;
using OperatorLiteral = details::TagToken<struct OperatorLiteralTag>;
using InvalidEncoding = details::TagToken<struct InvalidEncodingTag>;
using UnexpectedCharacter = details::TagToken<struct UnexpectedCharacterTag>;

using StringLiteral = details::ValueToken<StringLiteralValue>;
using NumberLiteral = details::ValueToken<NumberLiteralValue>;

using Token = meta::Variant<
    WhiteSpaceSeparator,
    NewLineIndentation,
    CommentLiteral,
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
    OperatorLiteral,
    InvalidEncoding,
    UnexpectedCharacter>;

using OptToken = meta::Optional<Token>;

} // namespace scanner
