#pragma once

#include "filter/Token.h"

#include "meta/CoEnumerator.h"
#include "meta/Unreachable.h"

namespace filter {

using ScannerToken = scanner::Token;
using ScannerTokenIndex = scanner::Token::Index;
using strings::View;

/**
 * @brief the token filter parser is the 1st parser step
 *
 * note:
 * • this buffers only one token O(n)
 *
 **/
inline auto filterTokens(meta::CoEnumerator<ScannerToken> input) -> meta::CoEnumerator<TokenLine> {
    auto translate = [](ScannerToken&& tok) -> Token {
        return std::move(tok).visit(
            [](scanner::CommentLiteral&&) { return meta::unreachable<Token>(); },
            [](scanner::WhiteSpaceSeparator&&) { return meta::unreachable<Token>(); },
            [](scanner::InvalidEncoding&&) { return meta::unreachable<Token>(); },
            [](scanner::UnexpectedCharacter&&) { return meta::unreachable<Token>(); },
            [](scanner::NewLineIndentation&&) { return meta::unreachable<Token>(); },
            [](scanner::SemicolonSeparator&&) { return meta::unreachable<Token>(); },
            [](auto&& d) { return Token{std::forward<decltype(d)>(d)}; });
    };
    auto line = TokenLine{};
    auto addInsignificant = [&](ScannerToken&& tok) {
        line.insignificants.push_back(std::move(tok).visit(
            [](scanner::CommentLiteral&& c) { return Insignificant{std::move(c)}; },
            [](scanner::WhiteSpaceSeparator&& c) { return Insignificant{c}; },
            [](scanner::InvalidEncoding&& c) { return Insignificant{c}; },
            [](scanner::UnexpectedCharacter&& c) { return Insignificant{c}; },
            [](scanner::SemicolonSeparator&& c) { return Insignificant{c}; },
            [&](scanner::NewLineIndentation&& c) {
                line.newLineIndex = line.insignificants.size();
                return Insignificant{c};
            },
            [&](scanner::ColonSeparator&& c) {
                line.blockStartColonIndex = line.insignificants.size();
                return Insignificant{c};
            },
            [&](scanner::IdentifierLiteral&& c) {
                line.blockEndIdentifierIndex = line.insignificants.size();
                return Insignificant{c};
            },
            [](auto&& d) { return meta::unreachable<Insignificant>(); }));
    };
    auto insertBlockStartColon = [&](size_t index, Insignificant colon) {
        if (colon.holds<ColonSeparator>()) line.blockStartColonIndex = index;
        line.insignificants.insert(line.insignificants.begin() + index, colon);
    };
    auto addToken = [&](Token&& tok) { line.tokens.emplace_back(std::move(tok)); };

    while (input++) {
        if (input->holds<scanner::NewLineIndentation, scanner::SemicolonSeparator>()) {
            if (line.isBlockEnd()) {
                co_yield std::move(line);
                line = TokenLine{};
            }
            addInsignificant(input.move());
            continue;
        }
        if (input->holds<
                scanner::CommentLiteral,
                scanner::WhiteSpaceSeparator,
                scanner::InvalidEncoding,
                scanner::UnexpectedCharacter>()) {
            addInsignificant(input.move());
            continue;
        }
        if (input->holds<scanner::IdentifierLiteral>()) {
            const auto& id = input->get<scanner::IdentifierLiteral>().input;
            if (line.startsOnNewLine() && id.isContentEqual(View{"end"})) {
                addInsignificant(input.move());
                continue; // ['\n' + "end"] => block end
            }
        }
        auto previous = translate(input.move());
        auto blockStartIndex = size_t{};
        if (previous.holds<ColonSeparator>()) blockStartIndex = line.insignificants.size();
        while (true) {
            if (!input++) {
                addToken(std::move(previous));
                co_yield std::move(line);
                co_return;
            }
            const auto& current = *input;
            if (current.holds<
                    scanner::CommentLiteral,
                    scanner::WhiteSpaceSeparator,
                    scanner::InvalidEncoding,
                    scanner::UnexpectedCharacter>()) {
                addInsignificant(input.move());
                continue; // skip insignificant tokens
            }
            if (current.holds<scanner::NewLineIndentation>()) {
                if (previous.holds<ColonSeparator>()) {
                    if (line.tokens.empty()) {
                        auto colon = previous.get<ColonSeparator>();
                        insertBlockStartColon(blockStartIndex, UnexpectedColon{colon});
                        co_yield std::move(line);
                        line = TokenLine{};
                        addInsignificant(input.move());
                        break; // error - ['\n' + ':' + '\n]
                    }
                    insertBlockStartColon(blockStartIndex, previous.get<ColonSeparator>());
                    co_yield std::move(line);
                    line = TokenLine{};
                    addInsignificant(input.move());
                    break; // [':' + '\n'] => block start
                }
                addToken(std::move(previous));
                co_yield std::move(line);
                line = TokenLine{};
                addInsignificant(input.move());
                break; // regular line end
            }
            if (current.holds<scanner::SemicolonSeparator>()) {
                addToken(std::move(previous));
                co_yield std::move(line);
                line = TokenLine{};
                addInsignificant(input.move());
                break; // line broken by semicolon
            }
            addToken(std::move(previous));
            previous = translate(input.move());
            if (previous.holds<ColonSeparator>()) blockStartIndex = line.insignificants.size();
        }
    }
    if (!line.tokens.empty() || !line.insignificants.empty()) co_yield line;
}

} // namespace filter
