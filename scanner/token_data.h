#pragma once
#include "number_literal.h"
#include "strings/rope.h"
#include "text_range.h"

namespace scanner {

using rope_t = strings::rope;

struct white_space_separator {
    bool operator==(const white_space_separator &) const { return true; }
    bool operator!=(const white_space_separator &o) const { return !(*this == o); }
};
struct new_line_indentation {
    bool operator==(const new_line_indentation &) const { return true; }
    bool operator!=(const new_line_indentation &o) const { return !(*this == o); }
};
struct block_start_indentation { // later composite
    bool operator==(const block_start_indentation &) const { return true; }
    bool operator!=(const block_start_indentation &o) const { return !(*this == o); }
};
struct block_end_indentation { // later composite
    bool operator==(const block_end_indentation &) const { return true; }
    bool operator!=(const block_end_indentation &o) const { return !(*this == o); }
};
struct comment_literal {
    bool operator==(const comment_literal &) const { return true; }
    bool operator!=(const comment_literal &o) const { return !(*this == o); }
};
struct colon_separator {
    bool operator==(const colon_separator &) const { return true; }
    bool operator!=(const colon_separator &o) const { return !(*this == o); }
};
struct comma_separator {
    bool operator==(const comma_separator &) const { return true; }
    bool operator!=(const comma_separator &o) const { return !(*this == o); }
};
struct semicolon_separator {
    bool operator==(const semicolon_separator &) const { return true; }
    bool operator!=(const semicolon_separator &o) const { return !(*this == o); }
};
struct square_bracket_open {
    bool operator==(const square_bracket_open &) const { return true; }
    bool operator!=(const square_bracket_open &o) const { return !(*this == o); }
};
struct square_bracket_close {
    bool operator==(const square_bracket_close &) const { return true; }
    bool operator!=(const square_bracket_close &o) const { return !(*this == o); }
};
struct bracket_open {
    bool operator==(const bracket_open &) const { return true; }
    bool operator!=(const bracket_open &o) const { return !(*this == o); }
};
struct bracket_close {
    bool operator==(const bracket_close &) const { return true; }
    bool operator!=(const bracket_close &o) const { return !(*this == o); }
};
struct string_literal {
    bool operator==(const string_literal &) const { return true; }
    bool operator!=(const string_literal &o) const { return !(*this == o); }
};
struct identifier_literal {
    rope_t content;
    rope_t splitted_from;
    bool left_separated = false;
    bool right_separated = false;

    bool operator==(const identifier_literal &o) const {
        return content == o.content && left_separated == o.left_separated && right_separated == o.right_separated;
    }
    bool operator!=(const identifier_literal &o) const { return !(*this == o); }
};

inline std::ostream &operator<<(std::ostream &out, const identifier_literal &ident) {
    out << (ident.left_separated ? "_" : "|") //
        << ident.content                      //
        << (ident.right_separated ? "_" : "|");
    return out;
}

struct operator_literal : identifier_literal {};
struct invalid_encoding { // input file is not encoded correctly
    bool operator==(const invalid_encoding &) const { return true; }
    bool operator!=(const invalid_encoding &o) const { return !(*this == o); }
};
struct unexpected_character { // character is not known to scanner / misplaced
    bool operator==(const unexpected_character &) const { return true; }
    bool operator!=(const unexpected_character &o) const { return !(*this == o); }
};

using token_variant =
    meta::variant<white_space_separator, new_line_indentation,
                  block_start_indentation, // later composite
                  block_end_indentation,   // later composite
                  comment_literal, colon_separator, comma_separator, semicolon_separator, square_bracket_open,
                  square_bracket_close, bracket_open, bracket_close, string_literal, number_literal_t,
                  identifier_literal, operator_literal, invalid_encoding, unexpected_character>;

inline std::ostream &operator<<(std::ostream &out, const token_variant &v) {
    v.visit([&](const white_space_separator &) { out << "<space>"; },
            [&](const new_line_indentation &) { out << "<\\n>"; },
            [&](const block_start_indentation &) { out << "<{>"; }, //
            [&](const block_end_indentation &) { out << "<}>"; },   //
            [&](const comment_literal &) { out << "<##>"; }, [&](const colon_separator &) { out << "<:>"; },
            [&](const comma_separator &) { out << "<,>"; }, [&](const semicolon_separator &) { out << "<;>"; },
            [&](const square_bracket_open &) { out << "<[>"; }, [&](const square_bracket_close &) { out << "<]>"; },
            [&](const bracket_open &) { out << "<(>"; }, [&](const bracket_close &) { out << "<)>"; },
            [&](const string_literal &str) { //
                out << "<\"\">";
            },
            [&](const number_literal_t &num) { //
                out << "<num: " << num << ">";
            },
            [&](const identifier_literal &ident) { //
                out << "<id: " << ident << ">";
            },
            [&](const operator_literal &op) { //
                out << "<op: " << op << ">";
            },
            [&](const invalid_encoding &) { out << "<E:enc>"; },    //
            [&](const unexpected_character &) { out << "<E:exp>"; } //
    );
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

} // namespace scanner
