#pragma once

#include "parser/prepared_token.h"

namespace parser::grouping {

namespace prepared = parser::prepared;

using view_t = strings::utf8_view;
using column_t = scanner::column_t;
using text_range = scanner::text_range;

struct block_literal;
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
};

using line = std::vector<token>;
struct block_literal {
    std::vector<line> lines;
};

std::ostream &operator<<(std::ostream &out, const token &v);

inline std::ostream &operator<<(std::ostream &out, const line &l) {
    for (const auto &tok : l) {
        out << tok;
    }
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const block_literal &b) {
    for (const auto &line : b.lines) {
        out << "  line: " << line << '\n';
    }
    return out;
}

inline std::ostream &operator<<(std::ostream &out, const token_variant &v) {
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

inline std::ostream &operator<<(std::ostream &out, const token &t) {
    out << t.range << t.data;
    return out;
}

} // namespace parser::grouping
