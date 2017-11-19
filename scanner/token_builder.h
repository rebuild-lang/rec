#pragma once
#include "token.h"

namespace scanner {

using tokens = std::vector<token>;

namespace details {

template<class Tok>
struct token_builder {
    static auto build(Tok &&t) -> token { // token type => token
        auto tok = token{};
        tok.data = std::move(t);
        return tok;
    }
};

template<>
struct token_builder<token> {
    static auto build(token &&t) -> token { return std::move(t); } // full token is kept
};

template<>
struct token_builder<view_t> {
    static auto build(const view_t &b) -> token {
        auto tok = token{};
        tok.range.text = b;
        tok.data = scanner::identifier_literal{};
        return tok;
    }
};

} // namespace details

template<class Tok>
auto build_token(Tok &&t) -> token {
    return details::token_builder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto build_tokens(Tok &&... t) -> tokens {
    return tokens{scanner::build_token(std::forward<Tok>(t))...};
}

} // namespace scanner
