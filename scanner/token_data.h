#pragma once
#include "number_literal.h"
#include "strings/rope.h"
#include "text_range.h"

namespace scanner {

using rope_t = strings::rope;

struct white_space_separator {};
struct new_line_indentation {};
struct block_start_indentation {}; // later composite
struct block_end_indentation {};   // later composite
struct comment_literal {};
struct colon_separator {};
struct comma_separator {};
struct semicolon_separator {};
struct square_bracket_open {};
struct square_bracket_close {};
struct bracket_open {};
struct bracket_close {};
struct string_literal {};
struct identifier_literal {
    rope_t content;
    rope_t splitted_from;
    bool left_separated = false;
    bool right_separated = false;
};
struct operator_literal : identifier_literal {};
struct invalid_encoding {};     // input file is not encoded correctly
struct unexpected_character {}; // character is not known to scanner / misplaced

using token_variant =
    meta::variant<white_space_separator, new_line_indentation,
                  block_start_indentation, // later composite
                  block_end_indentation,   // later composite
                  comment_literal, colon_separator, comma_separator, semicolon_separator, square_bracket_open,
                  square_bracket_close, bracket_open, bracket_close, string_literal, number_literal_t,
                  identifier_literal, operator_literal, invalid_encoding, unexpected_character>;

struct token_data {
    text_range range;
    token_variant data;

    template<class... Ts>
    bool one_of() const {
        return data.holds<Ts...>();
    }
};

} // namespace scanner
