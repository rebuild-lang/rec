#pragma once
#include "scanner/token.h"

namespace scanner {

using tokens = std::vector<token>;

namespace details {

template<class Tok>
struct token_builder {
    static token build(Tok &&t) { // token type => token
        auto tok = token{};
        tok.data = std::move(t);
        return tok;
    }
};

template<>
struct token_builder<token> {
    static token build(token &&t) { return std::move(t); } // full token is kept
};

template<>
struct token_builder<view_t> {
    static token build(const view_t &b) {
        auto tok = token{};
        tok.range.text = b;
        tok.data = scanner::identifier_literal{};
        return tok;
    }
};

} // namespace details

template<class Tok>
token build_token(Tok &&t) {
    return details::token_builder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
tokens build_tokens(Tok &&... t) {
    tokens result;
    (void)std::initializer_list<int>{(result.push_back(build_token(std::forward<Tok>(t))), 0)...};
    return result;
}

} // namespace scanner
