#pragma once

#include "parser/prepared_token.h"

namespace parser::grouping {

namespace prepared = parser::prepared;

using text_range = scanner::text_range;

struct token;
using line = std::vector<token>;
struct block_literal {
    std::vector<line> lines;

    bool operator==(const block_literal &o) const { return lines == o.lines; }
    bool operator!=(const block_literal &o) const { return lines != o.lines; }
};
using colon_separator = prepared::colon_separator;
using comma_separator = prepared::comma_separator;
using semicolon_separator = prepared::semicolon_separator;
using square_bracket_open = prepared::square_bracket_open;
using square_bracket_close = prepared::square_bracket_close;
using bracket_open = prepared::bracket_open;
using bracket_close = prepared::bracket_close;
using string_literal = prepared::string_literal;
using number_literal_t = prepared::number_literal_t;
using identifier_literal = prepared::identifier_literal;
using operator_literal = prepared::operator_literal;

using token_variant = meta::variant<
    block_literal,
    colon_separator,
    comma_separator,
    semicolon_separator,
    square_bracket_open,
    square_bracket_close,
    bracket_open,
    bracket_close,
    string_literal,
    number_literal_t,
    identifier_literal,
    operator_literal>;

struct token {
    text_range range;
    token_variant data;

    template<class... Ts>
    bool one_of() const {
        return data.holds<Ts...>();
    }

    bool operator==(const token &o) const { return range == o.range && data == o.data; }
    bool operator!=(const token &o) const { return !(*this == o); }
};

auto operator<<(std::ostream &out, const token &v) -> std::ostream &;

inline auto operator<<(std::ostream &out, const line &l) -> std::ostream & {
    for (const auto &tok : l) {
        out << tok;
    }
    return out;
}

inline auto operator<<(std::ostream &out, const block_literal &b) -> std::ostream & {
    for (const auto &line : b.lines) {
        out << "  line: " << line << '\n';
    }
    return out;
}

inline auto operator<<(std::ostream &out, const token_variant &v) -> std::ostream & {
    v.visit(
        [&](const block_literal &block) { out << "{block: " << block; },
        [&](const colon_separator &) { out << "<:>"; },
        [&](const comma_separator &) { out << "<,>"; },
        [&](const semicolon_separator &) { out << "<;>"; },
        [&](const square_bracket_open &) { out << "<[>"; },
        [&](const square_bracket_close &) { out << "<]>"; },
        [&](const bracket_open &) { out << "<(>"; },
        [&](const bracket_close &) { out << "<)>"; },
        [&](const string_literal &) { out << "<\"\">"; },
        [&](const number_literal_t &num) { out << "<num: " << num << ">"; },
        [&](const identifier_literal &ident) { out << "<id: " << ident << ">"; },
        [&](const operator_literal &op) { out << "<op: " << op << ">"; });
    return out;
}

inline auto operator<<(std::ostream &out, const token &t) -> std::ostream & {
    out << t.range << t.data;
    return out;
}

} // namespace parser::grouping
