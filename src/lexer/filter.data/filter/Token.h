#pragma once
#include "scanner/Token.h"

namespace filter {

using scanner::BracketClose;
using scanner::BracketOpen;
using scanner::CommaSeparator;
using scanner::IdentifierLiteral;
using scanner::NumberLiteral;
using scanner::OperatorLiteral;
using scanner::SquareBracketClose;
using scanner::SquareBracketOpen;
using scanner::StringLiteral;

using scanner::ColonSeparator;
using scanner::CommentLiteral;
using scanner::InvalidEncoding;
using scanner::NewLineIndentation;
using scanner::SemicolonSeparator;
using scanner::UnexpectedCharacter;
using scanner::WhiteSpaceSeparator;
using BlockStartColon = scanner::details::TagToken<struct BlockStartColonTag>;
using BlockEndIdentifier = scanner::details::TagToken<struct BlockEndIdentifierTag>;
using UnexpectedColon = scanner::details::TagErrorToken<struct UnexpectedColonTag>;

using Token = meta::Variant<
    ColonSeparator,
    CommaSeparator,
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
    SemicolonSeparator,
    NewLineIndentation,
    BlockStartColon,
    BlockEndIdentifier,
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
    bool startsOnNewLine() const { return newLineIndex != -1; }
    bool hasErrors() const {
        auto l = [](const auto& t) { return t.visit([](const auto& v) { return hasTokenError(v); }); };
        return std::any_of(tokens.begin(), tokens.end(), l) //
            || std::any_of(insignificants.begin(), insignificants.end(), l);
    }

    auto newLine() const -> const NewLineIndentation& { //
        return insignificants[newLineIndex].get<NewLineIndentation>();
    }
    auto blockStartColon() const -> const BlockStartColon& {
        return insignificants[blockStartColonIndex].get<BlockStartColon>();
    }
    auto blockEndIdentifier() const -> const BlockEndIdentifier& {
        return insignificants[blockEndIdentifierIndex].get<BlockEndIdentifier>();
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
