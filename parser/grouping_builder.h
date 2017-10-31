#pragma once

#include "parser/grouping_data.h"

namespace parser::grouping {

using tokens = std::vector<token>;
using column_t = scanner::column_t;
using view_t = scanner::view_t;

namespace details {

struct id_builder {
    token tok;

    inline id_builder(identifier_literal) { tok.data = identifier_literal{}; }
    inline id_builder(operator_literal) { tok.data = operator_literal{}; }
    id_builder(const id_builder &) = default;
    id_builder &operator=(const id_builder &) = default;
    id_builder(id_builder &&) = default;
    id_builder &operator=(id_builder &&) = default;

    inline auto lit() & -> identifier_literal & {
        if (tok.one_of<operator_literal>()) return tok.data.get<operator_literal>();
        return tok.data.get<identifier_literal>();
    }

    inline auto left_separated() && -> id_builder {
        lit().left_separated = true;
        return *this;
    }
    inline auto right_separated() && -> id_builder {
        lit().right_separated = true;
        return *this;
    }
    inline auto both_separated() && -> id_builder {
        lit().left_separated = true;
        lit().right_separated = true;
        return *this;
    }

    template<size_t N>
    auto text(const char (&text)[N]) && -> id_builder {
        tok.range.text = view_t{text};
        return *this;
    }

    inline auto id() && -> token && { return std::move(tok); }
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

template<class Tok>
auto build_token(Tok &&t) -> token {
    return details::token_builder<Tok>::build(std::forward<Tok>(t));
}

template<class... Tok>
auto build_tokens(Tok &&... t) -> tokens {
    tokens result;
    (void)std::initializer_list<int>{(result.push_back(parser::grouping::build_token(std::forward<Tok>(t))), 0)...};
    return result;
}

template<size_t N>
auto id(const char (&text)[N]) -> details::id_builder {
    return details::id_builder(identifier_literal{}).text(text);
}

template<size_t N>
auto op(const char (&text)[N]) -> details::id_builder {
    return details::id_builder(operator_literal{}).text(text);
}

auto block_start(column_t c) -> prepared::token {
    auto tok = prepared::token{};
    tok.range.end_position.column = c;
    tok.data = prepared::block_start_indentation{};
    return tok;
}
auto block_end(column_t c) -> prepared::token {
    auto tok = prepared::token{};
    tok.range.end_position.column = c;
    tok.data = prepared::block_end_indentation{};
    return tok;
}
auto new_line(column_t c) -> prepared::token {
    auto tok = prepared::token{};
    tok.range.end_position.column = c;
    tok.data = prepared::new_line_indentation{};
    return tok;
}

template<class... Lines>
auto block(column_t c, Lines &&... lines) -> token {
    auto tok = token{};
    tok.range.end_position.column = c;
    tok.data = block_literal{{std::forward<Lines>(lines)...}};
    return tok;
}

} // namespace parser::grouping
