#pragma once
#include "Token.h"

namespace scanner {

using Tokens = std::vector<Token>;

namespace details {

template<class Tok>
struct TokenBuilder {
    static auto build(Tok&& t) -> Token { // token type => token
        auto tok = Token{};
        tok.data = std::move(t);
        return tok;
    }
};

template<>
struct TokenBuilder<Token> {
    static auto build(Token&& t) -> Token { return std::move(t); } // full token is kept
};

template<>
struct TokenBuilder<View> {
    static auto build(const View& b) -> Token {
        auto tok = Token{};
        tok.range.text = b;
        tok.data = scanner::IdentifierLiteral{};
        return tok;
    }
};

} // namespace details

template<class Tok>
auto buildToken(Tok&& t) -> Token {
    return details::TokenBuilder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto buildTokens(Tok&&... t) -> Tokens {
    return Tokens{scanner::buildToken(std::forward<Tok>(t))...};
}

} // namespace scanner
