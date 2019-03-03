#pragma once
#include "NumberLiteralValue.h"
#include "OperatorLiteralValue.h"
#include "StringLiteralValue.h"

#include <text/DecodedPosition.h>

#include "meta/Optional.h"

namespace scanner {

using text::DecodedErrorPosition;
using text::InputPosition;
using DecodedErrorPositions = std::vector<DecodedErrorPosition>;

namespace details {

template<class... Tags>
using TagToken = InputPosition<struct TagTokenTag, Tags...>;

template<class... Tags>
constexpr auto hasTokenError(const TagToken<Tags...>&) {
    return false;
}

template<class... Tags>
using TagErrorToken = InputPosition<struct TagErrorTokenTag, Tags...>;

template<class... Tags>
constexpr auto hasTokenError(const TagErrorToken<Tags...>&) {
    return true;
}

template<class... Tags>
struct TagTokenWithDecodeErrors : text::InputPositionData {
    DecodedErrorPositions decodeErrors{};

    friend auto hasTokenError(const TagTokenWithDecodeErrors& t) { //
        return !t.decodeErrors.empty();
    }

    using This = TagTokenWithDecodeErrors;
    bool operator==(const This& o) const noexcept {
        return input == o.input && position == o.position && decodeErrors == o.decodeErrors;
    }
    bool operator!=(const This& o) const noexcept { return !(*this == o); }
};

template<class Value>
struct ValueToken : text::InputPositionData {
    Value value{};

    friend auto hasTokenError(const ValueToken& t) { //
        return t.value.hasErrors();
    }

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
using OperatorLiteral = details::ValueToken<OperatorLiteralValue>;

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
