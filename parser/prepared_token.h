#pragma once
#include "scanner/token.h"

namespace parser::prepared {

using text_range = scanner::text_range;
using view_t = scanner::view_t;

using new_line_indentation = scanner::new_line_indentation;
struct block_start_indentation {
    bool operator==(const block_start_indentation &) const { return true; }
    bool operator!=(const block_start_indentation &o) const { return !(*this == o); }
};
struct block_end_indentation {
    bool operator==(const block_end_indentation &) const { return true; }
    bool operator!=(const block_end_indentation &o) const { return !(*this == o); }
};
using colon_separator = scanner::colon_separator;
using comma_separator = scanner::comma_separator;
using semicolon_separator = scanner::semicolon_separator;
using square_bracket_open = scanner::square_bracket_open;
using square_bracket_close = scanner::square_bracket_close;
using bracket_open = scanner::bracket_open;
using bracket_close = scanner::bracket_close;
using string_literal = scanner::string_literal;
using number_literal_t = scanner::number_literal_t;
struct identifier_literal {
    bool left_separated = false;
    bool right_separated = false;

    bool operator==(const identifier_literal &o) const {
        return left_separated == o.left_separated && right_separated == o.right_separated;
    }
    bool operator!=(const identifier_literal &o) const { return !(*this == o); }
};
inline std::ostream &operator<<(std::ostream &out, const identifier_literal &ident) {
    return out << (ident.left_separated ? "_" : "|") << "T" << (ident.right_separated ? "_" : "|");
}
struct operator_literal : identifier_literal {};

using token_variant = meta::variant<
    new_line_indentation,
    block_start_indentation,
    block_end_indentation,
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

inline std::ostream &operator<<(std::ostream &out, const token_variant &v) {
    v.visit(
        [&](const new_line_indentation &) { out << "<\\n>"; },
        [&](const block_start_indentation &) { out << "<{>"; },
        [&](const block_end_indentation &) { out << "<}>"; },
        [&](const colon_separator &) { out << "<:>"; },
        [&](const comma_separator &) { out << "<,>"; },
        [&](const semicolon_separator &) { out << "<;>"; },
        [&](const square_bracket_open &) { out << "<[>"; },
        [&](const square_bracket_close &) { out << "<]>"; },
        [&](const bracket_open &) { out << "<(>"; },
        [&](const bracket_close &) { out << "<)>"; },
        [&](const string_literal &str) { out << "<\"\">"; },
        [&](const number_literal_t &num) { out << "<num: " << num << ">"; },
        [&](const identifier_literal &ident) { out << "<id: " << ident << ">"; },
        [&](const operator_literal &op) { out << "<op: " << op << ">"; });
    return out;
}

struct token {
    text_range range;
    token_variant data;

    template<class... Ts>
    bool one_of() const {
        return data.holds<Ts...>();
    }
};

inline std::ostream &operator<<(std::ostream &out, const token &t) {
    out << t.range << t.data;
    return out;
}

} // namespace parser::prepared
