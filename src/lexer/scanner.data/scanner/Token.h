#pragma once
#include "IdentifierLiteralValue.h"
#include "NewLineIndentationValue.h"
#include "NumberLiteralValue.h"
#include "StringLiteralValue.h"

#include <text/DecodedPosition.h>

#include "meta/Optional.h"

namespace scanner {

using text::DecodedErrorPosition;
using DecodedErrorPositions = std::vector<DecodedErrorPosition>;

namespace details {

template<class... Tags>
struct TagToken : text::InputPositionData {
    bool isTainted{};

    friend constexpr auto hasTokenError(const TagToken&) { return false; }
};

template<class... Tags>
struct TagErrorToken : text::InputPositionData {
    bool isTainted{}; // if an error is tainted it means the error is already reported

    friend constexpr auto hasTokenError(const TagErrorToken&) { return true; }
};

template<class... Tags>
struct TagTokenWithDecodeErrors : text::InputPositionData {
    using This = TagTokenWithDecodeErrors;

    DecodedErrorPositions decodeErrors{};
    bool isTainted{};

    friend auto hasTokenError(const This& t) { return !t.decodeErrors.empty(); }
    bool operator==(const This& o) const noexcept {
        return input == o.input && position == o.position && decodeErrors == o.decodeErrors;
    }
    bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

template<class Value>
struct ValueToken : text::InputPositionData {
    using This = ValueToken;

    Value value{};
    bool isTainted{};

    friend auto hasTokenError(const This& t) { return t.value.hasErrors(); }
    bool operator==(const This& o) const { return input == o.input && position == o.position && value == o.value; }
    bool operator!=(const This& o) const { return !(*this == o); }
};

} // namespace details

using WhiteSpaceSeparator = details::TagToken<struct WhiteSpaceSeparatorTag>;
using NewLineIndentation = details::ValueToken<NewLineIndentationValue>;
using CommentLiteral = details::TagTokenWithDecodeErrors<struct CommentLiteralTag>;
using ColonSeparator = details::TagToken<struct ColonSeparatorTag>;
using CommaSeparator = details::TagToken<struct CommaSeparatorTag>;
using SemicolonSeparator = details::TagToken<struct SemicolonSeparatorTag>;
using SquareBracketOpen = details::TagToken<struct SquareBracketOpenTag>;
using SquareBracketClose = details::TagToken<struct SquareBracketCloseTag>;
using BracketOpen = details::TagToken<struct BracketOpenTag>;
using BracketClose = details::TagToken<struct BracketCloseTag>;
using IdentifierLiteral = details::ValueToken<IdentifierLiteralValue>;

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
    InvalidEncoding,
    UnexpectedCharacter>;

using OptToken = meta::Optional<Token>;

} // namespace scanner
