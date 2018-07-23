#pragma once
#include "CommentScanner.h"
#include "IdentifierScanner.h"
#include "NumberScanner.h"
#include "OperatorScanner.h"
#include "StringScanner.h"

#include "scanner/Token.h"

#include "text/FileInput.h"

#include "meta/CoEnumerator.h"

namespace scanner {

using FileInput = text::FileInput;

struct Tokenizer {
    struct Config {
        text::Column tabStops; ///< columns per tabstop
    };
    explicit Tokenizer(Config c)
        : config(c) {}

    auto scanFile(const text::File& file) -> meta::CoEnumerator<Token> {
        auto input = FileInput(file);
        while (true) {
            input.collapse();
            const auto optCp = input.peek();
            if (!optCp) {
                if (!input.hasMorePeek()) co_return;
                co_yield scanInvalidEncoding(input);
                continue;
            }
            co_yield optCp.map([&](auto chr) -> Token {
                if (chr.isWhiteSpace()) return scanWhiteSpace(input);
                if (chr.isLineSeparator()) return scanNewLine(chr, input);
                if (chr.isDecimalNumber()) return scanNumberLiteral(chr, input);

                switch (chr.v) {
                case '"': return scanStringLiteral(input);
                case '#': return scanComment(input);
                case ':': return scanChar<ColonSeparator>(input);
                case ',': return scanChar<CommaSeparator>(input);
                case ';': return scanChar<SemicolonSeparator>(input);
                case '[': return scanChar<SquareBracketOpen>(input);
                case ']': return scanChar<SquareBracketClose>(input);
                case '(': return scanChar<BracketOpen>(input);
                case ')': return scanChar<BracketClose>(input);
                }

                if (auto optToken = scanIdentifier(input); optToken) return optToken.value();
                if (auto optToken = scanOperator(input); optToken) return optToken.value();
                return scanInvalid(input);
            });
        }
    }

private:
    static auto scanNumberLiteral(CodePoint chr, FileInput& input) -> Token { return NumberScanner::scan(chr, input); }
    static auto scanStringLiteral(FileInput& input) -> Token { return StringScanner::scan(input); }

    auto scanComment(FileInput& input) -> Token { return CommentScanner::scan(input, config.tabStops); }

    template<class Literal>
    static auto scanChar(FileInput& input) -> Token {
        input.extend();
        return Literal{input.range()};
    }

    static auto scanIdentifier(FileInput& input) -> OptToken { return IdentifierScanner::scan(input); }
    static auto scanOperator(FileInput& input) -> OptToken { return OperatorScanner::scan(input); }

    static auto scanInvalidEncoding(FileInput& input) -> Token {
        // invalid bytes were already skipped
        return InvalidEncoding{input.range()};
    }

    auto scanWhiteSpace(FileInput& input) const -> Token {
        input.extend(config.tabStops);
        input.extendWhiteSpaces(config.tabStops);
        return WhiteSpaceSeparator{input.range()};
    }

    auto scanNewLine(CodePoint chr, FileInput& input) const -> Token {
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
    }

    static auto scanInvalid(FileInput& input) -> Token {
        input.extend();
        return UnexpectedCharacter{input.range()};
    }

private:
    Config config;
};

} // namespace scanner
