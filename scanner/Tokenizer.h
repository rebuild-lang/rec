#pragma once

#include "meta/CoEnumerator.h"

#include "CommentScanner.h"
#include "FileInput.h"
#include "IdentifierScanner.h"
#include "NumberScanner.h"
#include "OperatorScanner.h"
#include "StringScanner.h"
#include "Token.h"

namespace scanner {

struct Tokenizer {
    struct Config {
        Column tabStops; ///< columns per tabstop
    };
    Tokenizer(Config c)
        : config(c) {}

    auto scanFile(const File& file) -> meta::CoEnumerator<Token> {
        auto input = FileInput(file);
        while (true) {
            input.collapse();
            const auto optCp = input.peek();
            if (!optCp) {
                if (!input.hasMorePeek()) co_return;
                co_yield scanInvalidEncoding(input);
                continue;
            }
            const auto chr = optCp.value();
            if (chr.isWhiteSpace()) {
                co_yield scanWhiteSpace(input);
                continue;
            }
            if (chr.isLineSeparator()) {
                co_yield scanNewLine(chr, input);
                continue;
            }
            if (chr.isDecimalNumber()) {
                co_yield scanNumberLiteral(chr, input);
                continue;
            }
            switch (chr.v) {
            case '"': co_yield scanStringLiteral(input); continue;
            case '#': co_yield scanComment(input); continue;
            case ':': co_yield scanChar<ColonSeparator>(input); continue;
            case ',': co_yield scanChar<CommaSeparator>(input); continue;
            case ';': co_yield scanChar<SemicolonSeparator>(input); continue;
            case '[': co_yield scanChar<SquareBracketOpen>(input); continue;
            case ']': co_yield scanChar<SquareBracketClose>(input); continue;
            case '(': co_yield scanChar<BracketOpen>(input); continue;
            case ')': co_yield scanChar<BracketClose>(input); continue;
            }
            auto identToken = scanIdentifier(input);
            if (identToken) {
                co_yield identToken.value();
                continue;
            }
            auto operatorToken = scanOperator(input);
            if (operatorToken) {
                co_yield operatorToken.value();
                continue;
            }
            co_yield scanInvalid(input);
        }
    }

private:
    static auto scanNumberLiteral(CodePoint chr, FileInput& input) -> Token { return NumberScanner::scan(chr, input); }
    static auto scanStringLiteral(FileInput& input) -> Token { return StringScanner::scan(input); }

    auto scanComment(FileInput& input) -> Token { return CommentScanner::scan(input, config.tabStops); }

    template<class Literal>
    static auto scanChar(FileInput& input) -> Token {
        input.extend();
        return {input.range(), Literal{}};
    }

    static auto scanIdentifier(FileInput& input) -> OptToken { return IdentifierScanner::scan(input); }
    static auto scanOperator(FileInput& input) -> OptToken { return OperatorScanner::scan(input); }

    static auto scanInvalidEncoding(FileInput& input) -> Token {
        // invalid bytes were already skipped
        return {input.range(), InvalidEncoding{}};
    }

    auto scanWhiteSpace(FileInput& input) const -> Token {
        input.extend(config.tabStops);
        input.extendWhiteSpaces(config.tabStops);
        return {input.range(), WhiteSpaceSeparator{}};
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
        return {input.range(), NewLineIndentation{}};
    }

    static auto scanInvalid(FileInput& input) -> Token {
        input.extend();
        return {input.range(), UnexpectedCharacter{}};
    }

private:
    Config config;
};

} // namespace scanner
