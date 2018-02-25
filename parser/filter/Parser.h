#pragma once

#include "parser/filter/Token.h"

#include "meta/CoEnumerator.h"

namespace parser::filter {

using ScannerToken = scanner::Token;
using ScannerTokenIndex = scanner::TokenVariant::Index;

/**
 * @brief the token filter parser is the 1st parser step
 *
 * filters out:
 * • invalid characters
 * • white spaces
 * • comments
 * • newline + indentations preceding comments
 * • two adjacent newline + indentations (basically multiple newlines)
 * • colon between two indentations (error)
 *
 * mutates:
 * • colon before new line & indentation => block start
 * • "end" after new line & indentation => block end
 * • identifiers separators around identifiers and operators
 *
 * note:
 * • this buffers only one token O(n)
 *
 **/
struct Parser {
    static auto parse(meta::CoEnumerator<ScannerToken> input) -> meta::CoEnumerator<Token> {
        while (true) {
            if (!input++) co_return;
            if (input->oneOf<
                    scanner::CommentLiteral,
                    scanner::WhiteSpaceSeparator,
                    scanner::InvalidEncoding,
                    scanner::UnexpectedCharacter>())
                continue; // skip initial values;
            break;
        }
        auto current = input.move();
        if (!current.oneOf<scanner::NewLineIndentation>()) {
            // ensure we always start with a new line
            co_yield Token{beforeRange(current), NewLineIndentation{}};
        }
        auto lastYieldType = TokenVariant::indexOf<NewLineIndentation>();
        auto previous = translate(std::move(current));
        if (previous.oneOf<IdentifierLiteral, OperatorLiteral>()) {
            markLeftSeparator(previous);
        }
        while (input++) {
            auto previousOrSkippedType = current.data.index();
            current = input.move();

            if (previous.oneOf<IdentifierLiteral, OperatorLiteral>()) {
                if (isRightSeparator(current)) markRightSeparator(previous);
            }
            if (previous.oneOf<ColonSeparator>()) {
                // this might be a block start, keep it in buffer
                while (current.oneOf<scanner::WhiteSpaceSeparator, scanner::CommentLiteral>()) {
                    if (!input++) {
                        co_yield previous;
                        co_return;
                    }
                    current = input.move();
                }
            }
            if (current.oneOf<
                    scanner::InvalidEncoding,
                    scanner::UnexpectedCharacter,
                    scanner::CommentLiteral,
                    scanner::WhiteSpaceSeparator>()) {
                continue; // skip these tokens
            }
            bool leftSeparated = false;
            if (current.oneOf<scanner::NewLineIndentation>()) {
                if (previous.oneOf<ColonSeparator>()) {
                    // if (lastYieldType.oneOf<NewLineIndentation, BlockStartIndentation>()) {
                    // not allowed
                    //}
                    // we do not merge the range here, because there might be skipped comments between
                    previous = Token{std::move(current.range), BlockStartIndentation{}};
                    continue; // [':' + '\n'] => block start
                }
                if (previous.oneOf<NewLineIndentation>()) {
                    // TODO: collapse range
                    continue; // skip second newline
                }
            }
            else if (current.oneOf<scanner::OperatorLiteral>()) {
                if (isLeftSeparator(previousOrSkippedType)) leftSeparated = true;
            }
            else if (current.oneOf<scanner::IdentifierLiteral>()) {
                if (previous.oneOf<NewLineIndentation, BlockStartIndentation>() &&
                    current.range.text.isContentEqual(View{"end"})) {
                    if (!previous.oneOf<NewLineIndentation>()) {
                        lastYieldType = previous.data.index();
                        co_yield previous;
                    }
                    previous = Token{previous.range, BlockEndIndentation{}};
                    continue; // ['\n' + "end"] => block end
                }
                if (isLeftSeparator(previousOrSkippedType)) leftSeparated = true;
            }
            lastYieldType = previous.data.index();
            co_yield previous;
            previous = translate(std::move(current));
            if (leftSeparated) markLeftSeparator(previous);
        }
        if (previous.oneOf<IdentifierLiteral, OperatorLiteral>()) markRightSeparator(previous);
        if (!previous.oneOf<NewLineIndentation>()) co_yield previous;
    }

private:
    constexpr static bool isRightSeparator(ScannerTokenIndex index) {
        using namespace scanner;
        return index.holds< //
            WhiteSpaceSeparator, // tok[\s]
            NewLineIndentation, // tok[\n]
            CommentLiteral, // tok#* *#
            ColonSeparator, // tok:
            CommaSeparator, // tok,
            SemicolonSeparator, // tok;
            SquareBracketClose, // tok]
            BracketClose // tok)
            >();
    }
    static bool isRightSeparator(const ScannerToken &t) { return isRightSeparator(t.data.index()); }

    constexpr static bool isLeftSeparator(ScannerTokenIndex index) {
        using namespace scanner;
        return index.holds< //
            WhiteSpaceSeparator, // [\s]tok
            NewLineIndentation, // [\n]tok
            CommentLiteral, // #* *#tok
            ColonSeparator, // :tok
            CommaSeparator, // ,tok
            SemicolonSeparator, // ;tok
            SquareBracketOpen, // [tok
            BracketOpen // (tok
            >();
    }

    static auto beforeRange(const ScannerToken &tok) -> TextRange { return {tok.range.file, {}, {}, tok.range.begin}; }

    static void markRightSeparator(Token &tok) {
        tok.data.visitSome(
            [](IdentifierLiteral &l) { l.rightSeparated = true; }, [](OperatorLiteral &o) { o.rightSeparated = true; });
    }

    static void markLeftSeparator(Token &tok) {
        tok.data.visitSome(
            [](IdentifierLiteral &l) { l.leftSeparated = true; }, [](OperatorLiteral &o) { o.leftSeparated = true; });
    }

    static auto translate(ScannerToken &&tok) -> Token {
        return std::move(tok.data).visit(
            [](scanner::WhiteSpaceSeparator &&) { return Token{}; },
            [](scanner::CommentLiteral &&) { return Token{}; },
            [](scanner::InvalidEncoding &&) { return Token{}; },
            [](scanner::UnexpectedCharacter &&) { return Token{}; },
            [&](scanner::IdentifierLiteral &&) {
                return Token{std::move(tok.range), IdentifierLiteral{}};
            },
            [&](scanner::OperatorLiteral &&) {
                return Token{std::move(tok.range), OperatorLiteral{}};
            },
            [&](auto &&d) {
                return Token{std::move(tok.range), std::move(d)};
            });
    }
};

} // namespace parser::filter
