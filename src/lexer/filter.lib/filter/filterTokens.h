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
            [](auto&& d) { return Token{std::forward<decltype(d)>(d)}; });
    };
    auto line = TokenLine{};
    auto addInsignificant = [&](ScannerToken&& tok) {
        line.insignificants.push_back(std::move(tok).visit(
            [](scanner::CommentLiteral&& c) { return Insignificant{std::move(c)}; },
            [](scanner::WhiteSpaceSeparator&& c) { return Insignificant{c}; },
            [](scanner::InvalidEncoding&& c) { return Insignificant{c}; },
            [](scanner::UnexpectedCharacter&& c) { return Insignificant{c}; },
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
    auto addToken = [&](Token&& tok) { line.tokens.emplace_back(std::move(tok)); };

    while (input++) {
        if (input->holds<
                scanner::CommentLiteral,
                scanner::WhiteSpaceSeparator,
                scanner::InvalidEncoding,
                scanner::UnexpectedCharacter,
                scanner::NewLineIndentation>()) {
            addInsignificant(input.move());
            continue;
        }
        auto previous = translate(input.move());
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
                        line.insignificants.emplace_back(UnexpectedColon{colon});
                        co_yield std::move(line);
                        line = TokenLine{};
                        addInsignificant(input.move());
                        break; // error - ['\n' + ':' + '\n]
                    }
                    co_yield std::move(line);
                    line = TokenLine{};
                    addInsignificant(previous.get<ColonSeparator>());
                    addInsignificant(input.move());
                    break; // [':' + '\n'] => block start
                }
                addToken(std::move(previous));
                co_yield std::move(line);
                line = TokenLine{};
                addInsignificant(input.move());
                break; // regular line end
            }
            if (current.holds<scanner::IdentifierLiteral>()) {
                const auto& id = current.get<scanner::IdentifierLiteral>().input;
                if (line.tokens.empty() && id.isContentEqual(View{"end"})) {
                    addInsignificant(input.move());
                    continue; // ['\n' + "end"] => block end
                }
            }
            addToken(std::move(previous));
            previous = translate(input.move());
        }
    }
    if (!line.tokens.empty() || !line.insignificants.empty()) co_yield line;
}

} // namespace filter
