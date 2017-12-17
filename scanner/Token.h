#pragma once
#include "NumberLiteral.h"
#include "TextRange.h"

namespace scanner {

struct WhiteSpaceSeparator {
    bool operator==(const WhiteSpaceSeparator &) const { return true; }
    bool operator!=(const WhiteSpaceSeparator &o) const { return !(*this == o); }
};
struct NewLineIndentation {
    bool operator==(const NewLineIndentation &) const { return true; }
    bool operator!=(const NewLineIndentation &o) const { return !(*this == o); }
};
struct CommentLiteral {
    bool operator==(const CommentLiteral &) const { return true; }
    bool operator!=(const CommentLiteral &o) const { return !(*this == o); }
};
struct ColonSeparator {
    bool operator==(const ColonSeparator &) const { return true; }
    bool operator!=(const ColonSeparator &o) const { return !(*this == o); }
};
struct CommaSeparator {
    bool operator==(const CommaSeparator &) const { return true; }
    bool operator!=(const CommaSeparator &o) const { return !(*this == o); }
};
struct SemicolonSeparator {
    bool operator==(const SemicolonSeparator &) const { return true; }
    bool operator!=(const SemicolonSeparator &o) const { return !(*this == o); }
};
struct SquareBracketOpen {
    bool operator==(const SquareBracketOpen &) const { return true; }
    bool operator!=(const SquareBracketOpen &o) const { return !(*this == o); }
};
struct SquareBracketClose {
    bool operator==(const SquareBracketClose &) const { return true; }
    bool operator!=(const SquareBracketClose &o) const { return !(*this == o); }
};
struct BracketOpen {
    bool operator==(const BracketOpen &) const { return true; }
    bool operator!=(const BracketOpen &o) const { return !(*this == o); }
};
struct BracketClose {
    bool operator==(const BracketClose &) const { return true; }
    bool operator!=(const BracketClose &o) const { return !(*this == o); }
};
struct StringLiteral {
    bool operator==(const StringLiteral &) const { return true; }
    bool operator!=(const StringLiteral &o) const { return !(*this == o); }
};
struct IdentifierLiteral {
    bool operator==(const IdentifierLiteral &) const { return true; }
    bool operator!=(const IdentifierLiteral &o) const { return !(*this == o); }
};
struct OperatorLiteral {
    bool operator==(const OperatorLiteral &) const { return true; }
    bool operator!=(const OperatorLiteral &o) const { return !(*this == o); }
};
struct InvalidEncoding { // input file is not encoded correctly
    bool operator==(const InvalidEncoding &) const { return true; }
    bool operator!=(const InvalidEncoding &o) const { return !(*this == o); }
};
struct UnexpectedCharacter { // character is not known to scanner / misplaced
    bool operator==(const UnexpectedCharacter &) const { return true; }
    bool operator!=(const UnexpectedCharacter &o) const { return !(*this == o); }
};

using TokenVariant = meta::Variant<
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

struct Token {
    TextRange range;
    TokenVariant data;

    template<class... Ts>
    bool oneOf() const {
        return data.holds<Ts...>();
    }
};

} // namespace scanner
