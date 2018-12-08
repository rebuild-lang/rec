#pragma once
#include "NumberLiteralValue.h"
#include "StringLiteralValue.h"

#include "text/Range.h"
#include <text/DecodedPosition.h>

#include "meta/Optional.h"

namespace scanner {

using text::DecodedErrorPosition;
using text::Position;
using text::View;
using DecodedErrorPositions = std::vector<DecodedErrorPosition>;

namespace details {

template<class Tag>
struct TagToken {
    View input{};
    Position position{};

    explicit operator bool() const { return true; }

    using This = TagToken;
    bool operator==(const This& o) const { return true; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
template<class Tag>
struct TagErrorToken {
    View input{};
    Position position{};

    explicit operator bool() const { return false; }

    using This = TagErrorToken;
    bool operator==(const This& o) const { return true; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
template<class Tag>
struct TagTokenWithDecodeErrors {
    View input{};
    Position position{};
    DecodedErrorPositions decodeErrors{};

    explicit operator bool() const { return decodeErrors.empty(); }

    using This = TagTokenWithDecodeErrors;
    bool operator==(const This& o) const noexcept { return decodeErrors == o.decodeErrors; }
    bool operator!=(const This& o) const noexcept { return !(*this == o); }
};
template<class Value>
struct ValueToken {
    View input{};
    Position position{};
    Value value{};

    explicit operator bool() const { return value; }

    using This = ValueToken;
    bool operator==(const This& o) const { return value == o.value; }
    bool operator!=(const This& o) const { return !(*this == o); }
};
} // namespace details

using WhiteSpaceSeparator = details::TagToken<struct WhiteSpaceSeparatorTag>;
using NewLineIndentation = details::TagToken<struct NewLineIndentationTag>;
using CommentLiteral = details::TagTokenWithDecodeErrors<struct CommentLiteralTag>;
using ColonSeparator = details::TagToken<struct ColonSeparatorTag>;
using CommaSeparator = details::TagToken<struct CommaSeparatorTag>;
using SemicolonSeparator = details::TagToken<struct SemicolonSeparatorTag>;
using SquareBracketOpen = details::TagToken<struct SquareBracketOpenTag>;
using SquareBracketClose = details::TagToken<struct SquareBracketCloseTag>;
using BracketOpen = details::TagToken<struct BracketOpenTag>;
using BracketClose = details::TagToken<struct BracketCloseTag>;
using IdentifierLiteral = details::TagToken<struct IdentifierLiteralTag>;
using OperatorLiteral = details::TagTokenWithDecodeErrors<struct OperatorLiteralTag>;

// UTF8-Decoder found a problem
using InvalidEncoding = details::TagErrorToken<struct InvalidEncodingTag>;

// Valid Codepoint but not part of any valid Token
using UnexpectedCharacter = details::TagErrorToken<struct UnexpectedCharacterTag>;

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
