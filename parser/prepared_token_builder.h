#pragma once

#include "parser/prepared_token.h"

namespace parser::prepared {

using tokens = std::vector<token>;

namespace details {

struct id_builder {
    token tok;

    id_builder() { tok.data = identifier_literal{}; }
    id_builder(const id_builder &) = default;
    id_builder &operator=(const id_builder &) = default;
    id_builder(id_builder &&) = default;
    id_builder &operator=(id_builder &&) = default;

    auto lit() & -> identifier_literal & { return tok.data.get<identifier_literal>(); }

    auto left_separated() && -> id_builder {
        lit().left_separated = true;
        return *this;
    }
    auto right_separated() && -> id_builder {
        lit().right_separated = true;
        return *this;
    }
    auto both_separated() && -> id_builder {
        lit().left_separated = true;
        lit().right_separated = true;
        return *this;
    }

    template<size_t N>
    auto text(const char (&text)[N]) && -> id_builder {
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

auto id() -> details::id_builder { return {}; }

template<size_t N>
auto id(const char (&text)[N]) -> details::id_builder {
    return details::id_builder{}.text(text);
}

template<class Tok>
auto build_token(Tok &&t) -> token {
    return details::token_builder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto build_tokens(Tok &&... t) -> tokens {
    tokens result;
    (void)std::initializer_list<int>{(result.push_back(parser::prepared::build_token(std::forward<Tok>(t))), 0)...};
    return result;
}

} // namespace parser::prepared
