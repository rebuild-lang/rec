#pragma once

#include "filter/Token.h"

#include "meta/CoEnumerator.h"
#include "meta/Unreachable.h"

namespace filter {

using ScannerToken = scanner::Token;
using ScannerTokenIndex = scanner::Token::Index;

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
inline auto filterTokens(meta::CoEnumerator<ScannerToken> input) -> meta::CoEnumerator<Token> {
    auto isRightSeparatorByIndex = [](ScannerTokenIndex index) -> bool {
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
    };
    auto isRightSeparator = [&](const ScannerToken& t) -> bool { return isRightSeparatorByIndex(t.index()); };

    auto isLeftSeparator = [](ScannerTokenIndex index) -> bool {
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
    };

    auto beforeRange = [](const ScannerToken& tok) -> TextRange {
        // return tok.visit([](auto& t) -> TextRange { return {t.range.file, {}, {}, t.range.begin}; });
        return {};
    };

    auto markRightSeparator = [](Token& tok) {
        tok.visitSome(
            [](IdentifierLiteral& l) { l.value.rightSeparated = true; }, //
            [](OperatorLiteral& o) { o.value.rightSeparated = true; });
    };

    auto markLeftSeparator = [](Token& tok) {
        tok.visitSome(
            [](IdentifierLiteral& l) { l.value.leftSeparated = true; }, //
            [](OperatorLiteral& o) { o.value.leftSeparated = true; });
    };

    auto translate = [](ScannerToken&& tok) -> Token {
        return std::move(tok).visit(
            [](scanner::WhiteSpaceSeparator&&) { return meta::unreachable<Token>(); },
            [](scanner::CommentLiteral&&) { return meta::unreachable<Token>(); },
            [](scanner::InvalidEncoding&&) { return meta::unreachable<Token>(); },
            [](scanner::UnexpectedCharacter&&) { return meta::unreachable<Token>(); },
            [](scanner::IdentifierLiteral&& id) -> Token {
                return {}; // IdentifierLiteral{{}, id.range};
            },
            [](scanner::OperatorLiteral&& op) -> Token {
                return {}; // OperatorLiteral{{{}, op.range}};
            },
            [](auto&& d) { return Token{std::move(d)}; });
    };

    while (true) {
        if (!input++) co_return;
        if (input->holds<
                scanner::CommentLiteral,
                scanner::WhiteSpaceSeparator,
                scanner::InvalidEncoding,
                scanner::UnexpectedCharacter>())
            continue; // skip initial values;
        break;
    }
    auto current = input.move();
    if (!current.holds<scanner::NewLineIndentation>()) {
        // ensure we always start with a new line
        // co_yield NewLineIndentation{beforeRange(current)};
    }
    auto lastYieldType = Token::indexOf<NewLineIndentation>();
    auto previous = translate(std::move(current));
    if (previous.holds<IdentifierLiteral, OperatorLiteral>()) {
        markLeftSeparator(previous);
    }
    while (input++) {
        auto previousOrSkippedType = current.index();
        current = input.move();

        if (previous.holds<IdentifierLiteral, OperatorLiteral>()) {
            if (isRightSeparator(current)) markRightSeparator(previous);
        }
        if (previous.holds<ColonSeparator>()) {
            // this might be a block start, keep it in buffer
            while (current.holds<scanner::WhiteSpaceSeparator, scanner::CommentLiteral>()) {
                if (!input++) {
                    co_yield previous;
                    co_return;
                }
                current = input.move();
            }
        }
        if (current.holds<
                scanner::InvalidEncoding,
                scanner::UnexpectedCharacter,
                scanner::CommentLiteral,
                scanner::WhiteSpaceSeparator>()) {
            continue; // skip these tokens
        }
        bool leftSeparated = false;
        if (current.holds<scanner::NewLineIndentation>()) {
            if (previous.holds<ColonSeparator>()) {
                // if (lastYieldType.oneOf<NewLineIndentation, BlockStartIndentation>()) {
                // TODO(arBmind): report error (not allowed)
                // }
                // we do not merge the range here, because there might be skipped comments between
                // previous = BlockStartIndentation{std::move(current.get<scanner::NewLineIndentation>().range)};
                continue; // [':' + '\n'] => block start
            }
            if (previous.holds<NewLineIndentation>()) {
                // TODO(arBmind): collapse range
                continue; // skip second newline
            }
        }
        else if (current.holds<scanner::OperatorLiteral>()) {
            if (isLeftSeparator(previousOrSkippedType)) leftSeparated = true;
        }
        else if (current.holds<scanner::IdentifierLiteral>()) {
            const auto& id = current.get<scanner::IdentifierLiteral>();
            /*if (previous.holds<NewLineIndentation>() && id.range.view.isContentEqual(View{"end"})) {
                previous = BlockEndIndentation{id.range};
                continue; // ['\n' + "end"] => block end
            }
            if (previous.holds<BlockStartIndentation>() && id.range.view.isContentEqual(View{"end"})) {
                lastYieldType = previous.index();
                co_yield previous;
                previous = BlockEndIndentation{id.range};
                continue; // [':\n' + "end"] => block begin + block end
            }*/
            if (isLeftSeparator(previousOrSkippedType)) leftSeparated = true;
        }
        lastYieldType = previous.index();
        co_yield previous;
        previous = translate(std::move(current));
        if (leftSeparated) markLeftSeparator(previous);
    }
    if (previous.holds<IdentifierLiteral, OperatorLiteral>()) markRightSeparator(previous);
    if (!previous.holds<NewLineIndentation>()) co_yield previous;
}

} // namespace filter
