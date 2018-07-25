#pragma once
#include "CommentScanner.h"
#include "IdentifierScanner.h"
#include "NumberScanner.h"
#include "OperatorScanner.h"
#include "StringScanner.h"

#include "scanner/Token.h"

#include "text/FileInput.h"

#include "meta/CoEnumerator.h"
#include "meta/TypeList.h"

namespace scanner {

struct Config {
    text::Column tabStops{}; ///< columns per tabstop
};

inline auto tokensFromFile(const text::File& file, Config config) -> meta::CoEnumerator<Token> {
    auto input = text::FileInput{file};

    auto invalidEncoding = [&]() -> Token {
        // note: invalid bytes were already skipped
        return InvalidEncoding{input.range()};
    };
    auto whiteSpace = [&]() -> Token {
        input.extend(config.tabStops);
        input.extendWhiteSpaces(config.tabStops);
        return WhiteSpaceSeparator{input.range()};
    };
    auto newLine = [&](CodePoint chr) -> Token {
        input.skip();
        // skip 2nd char of newline pair
        if (chr == '\n' || chr == '\r') {
            input.peek().map([&](auto chr2) {
                if (chr2 != chr && (chr2 == '\n' || chr2 == '\r')) input.skip();
            });
        }
        input.nextLine();
        input.extendWhiteSpaces(config.tabStops);
        return NewLineIndentation{input.range()};
    };
    auto numberLiteral = [&](CodePoint chr) -> Token { return extractNumber(chr, input); };
    auto stringLiteral = [&]() -> Token { return StringScanner::scan(input); };
    auto comment = [&]() -> Token { return extractComment(input, config.tabStops); };
    auto charToken = [&](auto tokenType) -> Token {
        using TokenType = meta::unwrapType<decltype(tokenType)>;
        input.extend();
        return TokenType{input.range()};
    };
    auto identifier = [&]() -> OptToken { return IdentifierScanner::scan(input); };
    auto operatorT = [&]() -> OptToken { return OperatorScanner::scan(input); };
    auto invalid = [&]() -> Token {
        input.extend();
        return UnexpectedCharacter{input.range()};
    };

    while (true) {
        input.collapse();
        const auto optCp = input.peek();
        if (!optCp) {
            if (!input.hasMorePeek()) co_return; // end of file
            co_yield invalidEncoding();
        }
        else {
            co_yield optCp.map([&](auto chr) -> Token {
                if (chr.isWhiteSpace()) return whiteSpace();
                if (chr.isLineSeparator()) return newLine(chr);
                if (chr.isDecimalNumber()) return numberLiteral(chr);

                switch (chr.v) {
                case '"': return stringLiteral();
                case '#': return comment();
                case ':': return charToken(meta::Type<ColonSeparator>{});
                case ',': return charToken(meta::Type<CommaSeparator>{});
                case ';': return charToken(meta::Type<SemicolonSeparator>{});
                case '[': return charToken(meta::Type<SquareBracketOpen>{});
                case ']': return charToken(meta::Type<SquareBracketClose>{});
                case '(': return charToken(meta::Type<BracketOpen>{});
                case ')': return charToken(meta::Type<BracketClose>{});
                }

                if (auto optToken = identifier(); optToken) return optToken.value();
                if (auto optToken = operatorT(); optToken) return optToken.value();
                return invalid();
            });
        }
    }
}

} // namespace scanner
