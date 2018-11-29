#pragma once
#include "CommentLiteralValue.h"
#include "NumberLiteralValue.h"
#include "StringLiteralValue.h"

#include "text/Range.h"

#include "meta/Optional.h"

namespace scanner {

using text::Position;
using text::View;

namespace details {

template<class Tag>
struct TagToken {
    View input{};
    Position position{};

    using This = TagToken;
    bool operator==(const This& o) const { return true; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
template<class Value>
struct ValueToken {
    Value value{};
    View input{};
    Position position{};

    using This = ValueToken;
    bool operator==(const This& o) const { return value == o.value; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
} // namespace details

using WhiteSpaceSeparator = details::TagToken<struct WhiteSpaceSeparatorTag>;
using NewLineIndentation = details::TagToken<struct NewLineIndentationTag>;
using ColonSeparator = details::TagToken<struct ColonSeparatorTag>;
using CommaSeparator = details::TagToken<struct CommaSeparatorTag>;
using SemicolonSeparator = details::TagToken<struct SemicolonSeparatorTag>;
using SquareBracketOpen = details::TagToken<struct SquareBracketOpenTag>;
using SquareBracketClose = details::TagToken<struct SquareBracketCloseTag>;
using BracketOpen = details::TagToken<struct BracketOpenTag>;
using BracketClose = details::TagToken<struct BracketCloseTag>;
using IdentifierLiteral = details::TagToken<struct IdentifierLiteralTag>;
using OperatorLiteral = details::TagToken<struct OperatorLiteralTag>;

// UTF8-Decoder found a problem
using InvalidEncoding = details::TagToken<struct InvalidEncodingTag>;

// Valid Codepoint but not part of any valid Token
using UnexpectedCharacter = details::TagToken<struct UnexpectedCharacterTag>;

using CommentLiteral = details::ValueToken<CommentLiteralValue>;
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
