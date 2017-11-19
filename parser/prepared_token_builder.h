#pragma once

#include "parser/prepared_token.h"

namespace parser::prepared {

using tokens = std::vector<token>;

namespace details {

struct id_builder {
    using this_t = id_builder;
    token tok;

    id_builder(identifier_literal) { tok.data = identifier_literal{}; }
    id_builder(operator_literal) { tok.data = operator_literal{}; }
    id_builder(const this_t &) = default;
    this_t &operator=(const this_t &) = default;
    id_builder(this_t &&) = default;
    this_t &operator=(this_t &&) = default;

    auto lit() & -> identifier_literal & {
        if (tok.one_of<operator_literal>()) return tok.data.get<operator_literal>();
        return tok.data.get<identifier_literal>();
    }

    auto left_separated() && -> this_t {
        lit().left_separated = true;
        return *this;
    }
    auto right_separated() && -> this_t {
        lit().right_separated = true;
        return *this;
    }
    auto both_separated() && -> this_t {
        lit().left_separated = true;
        lit().right_separated = true;
        return *this;
    }

    template<size_t N>
    auto text(const char (&text)[N]) && -> this_t {
        tok.range.text = view_t{text};
        return *this;
    }

    auto id() && -> token && { return std::move(tok); }
};

template<class Tok>
struct token_builder {
    static auto build(Tok &&t) -> token {
        auto tok = token{};
        tok.data = std::move(t);
        return tok;
    }
};
template<>
struct token_builder<token> {
    static auto build(token &&t) -> token { return std::move(t); }
};
template<>
struct token_builder<id_builder> {
    static auto build(id_builder &&b) -> token { return std::move(b).id(); }
};

} // namespace details

inline auto id() -> details::id_builder { return {identifier_literal{}}; }

template<size_t N>
auto id(const char (&text)[N]) -> details::id_builder {
    return details::id_builder(identifier_literal{}).text(text);
}

template<size_t N>
auto op(const char (&text)[N]) -> details::id_builder {
    return details::id_builder(operator_literal{}).text(text);
}

template<class Tok>
auto build_token(Tok &&t) -> token {
    return details::token_builder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto build_tokens(Tok &&... t) -> tokens {
    return tokens{parser::prepared::build_token(std::forward<Tok>(t))...};
}

} // namespace parser::prepared
