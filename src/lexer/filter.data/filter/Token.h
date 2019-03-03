#pragma once
#include "scanner/Token.h"

namespace filter {

using scanner::BracketClose;
using scanner::BracketOpen;
using scanner::ColonSeparator;
using scanner::CommaSeparator;
using scanner::CommentLiteral;
using scanner::IdentifierLiteral;
using scanner::InvalidEncoding;
using scanner::NewLineIndentation;
using scanner::NumberLiteral;
using scanner::OperatorLiteral;
using scanner::SemicolonSeparator;
using scanner::SquareBracketClose;
using scanner::SquareBracketOpen;
using scanner::StringLiteral;
using scanner::UnexpectedCharacter;
using scanner::WhiteSpaceSeparator;
using UnexpectedColon = scanner::details::TagErrorToken<struct UnexpectedColonTag>;

using Token = meta::Variant<
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

using Insignificant = meta::Variant<
    CommentLiteral,
    WhiteSpaceSeparator,
    InvalidEncoding,
    UnexpectedCharacter,
    NewLineIndentation,
    ColonSeparator, // block start colon
    IdentifierLiteral, // block "end" identifier
    UnexpectedColon>;

struct TokenLine {
    using This = TokenLine;
    std::vector<Token> tokens{};

    std::vector<Insignificant> insignificants;
    int newLineIndex{-1}; // used for indentation
    int blockStartColonIndex{-1};
    int blockEndIdentifierIndex{-1};

    bool isBlockStart() const { return blockStartColonIndex != -1; }
    bool isBlockEnd() const { return blockEndIdentifierIndex != -1; }
    bool hasErrors() const {
        auto l = [](const auto& t) { return t.visit([](const auto& v) { return hasTokenError(v); }); };
        return std::any_of(tokens.begin(), tokens.end(), l) //
            || std::any_of(insignificants.begin(), insignificants.end(), l);
    }

    template<class F>
    void forEach(F&& f) const {
        auto ti = tokens.begin();
        auto te = tokens.end();
        auto ii = insignificants.begin();
        auto ie = insignificants.end();
        while (ti != te && ii != ie) {
            auto tv = ti->visit([](auto& x) { return x.input; });
            auto iv = ii->visit([](auto& x) { return x.input; });
            if (tv.begin() < iv.begin()) {
                f(*ti++);
            }
            else {
                f(*ii++);
            }
        }
        while (ti != te) f(*ti++);
        while (ii != ie) f(*ii++);
    }

    bool operator==(const This& o) const {
        return tokens == o.tokens //
            && insignificants == o.insignificants //
            && newLineIndex == o.newLineIndex //
            && blockStartColonIndex == o.blockStartColonIndex //
            && blockEndIdentifierIndex == o.blockEndIdentifierIndex;
    }
    bool operator!=(const This& o) const { return !(*this == o); }
};

} // namespace filter
