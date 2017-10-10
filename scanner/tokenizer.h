#pragma once

#include "meta/co_enumerator.h"
#include "meta/optional.h"
#include "meta/variant.h"
#include "strings/code_point.h"
#include "strings/utf8_string.h"
#include "strings/utf8_view.h"

#include <deque>

namespace scanner {

using string_t = strings::utf8_string;
using view_t = strings::utf8_view;
using char_t = strings::code_point_t;
using opt_char_t = strings::optional_code_point_t;

struct file_t {
    string_t filename;
    string_t content;
};

struct line_t {
    uint32_t v;

    constexpr line_t()
        : v(1) {} // start with line 1

    constexpr line_t(const line_t &) = default;
    constexpr line_t &operator=(const line_t &) = default;
    constexpr line_t(line_t &&) = default;
    constexpr line_t &operator=(line_t &&) = default;

    constexpr line_t &operator++() noexcept {
        v++;
        return *this;
    }
};

struct column_t {
    uint32_t v;

    constexpr column_t()
        : v(1) {} // start with column 1
    constexpr explicit column_t(uint32_t x)
        : v(x) {}

    constexpr column_t(const column_t &) = default;
    constexpr column_t &operator=(const column_t &) = default;
    constexpr column_t(column_t &&) = default;
    constexpr column_t &operator=(column_t &&) = default;

    constexpr column_t &operator++() noexcept {
        v++;
        return *this;
    }
};

struct position_t {
    line_t line;
    column_t column;

    constexpr void next_column() noexcept { ++column; }
    constexpr void next_tabstop(column_t tabstop) noexcept { column.v += tabstop.v - (column.v % tabstop.v); }
    constexpr void next_line() noexcept {
        ++line;
        column = {};
    }
};

struct text_range {
    const file_t *file;
    view_t text;
    position_t begin_position;
    position_t end_position;
};

struct file_input_t {
    using value_ptr = const string_t::value_type *;

    file_input_t(const file_t &file)
        : file_ptr(&file)
        , begin_ptr(file.content.data())
        , current_ptr(file.content.data())
        , peek_ptr(file.content.data())
        , begin_position({})
        , current_position({}) {}

    bool has_more_peek() const { return peek_ptr != end_ptr(); }

    auto range() const {
        return text_range{file_ptr, view_t(begin_ptr, current_ptr), begin_position, current_position};
    }

    void collapse() {
        begin_ptr = current_ptr;
        begin_position = current_position;
    }

    void rollback() {
        current_ptr = begin_ptr;
        current_position = begin_position;
        peek_ptr = current_ptr;
        peek_buffer.clear();
    }

    template<size_t index = 0>
    opt_char_t peek_char() {
        if (peek_buffer.size() <= index) {
            if (!fill_peek(index + 1)) return {};
        }
        return peek_buffer[index];
    }

    bool skip();

    bool extend(column_t tabstop = {});

    bool extend(size_t count) {
        for (auto i = 0u; i < count; ++i)
            if (!extend()) return false;
        return true;
    }

    void extend_white_spaces(column_t tabstop) {
        while (peek_char().map([=](char_t chr) {
            if (!chr.is_white_space()) return false;
            extend(tabstop);
            return true;
        }))
            ;
    }

    void collapse_white_spaces(column_t tabstop) {
        extend_white_spaces(tabstop);
        collapse();
    }

    void next_line() { current_position.next_line(); }

private:
    value_ptr end_ptr() const { return file_ptr->content.data() + file_ptr->content.byte_count().v; }
    bool fill_peek(size_t count);

private:
    const file_t *file_ptr;
    value_ptr begin_ptr;
    value_ptr current_ptr;
    value_ptr peek_ptr;
    position_t begin_position;
    position_t current_position;
    std::deque<char_t> peek_buffer;
};

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
struct number_literal {};
struct identifier_literal {};
struct operator_literal : identifier_literal {};
struct invalid_encoding {};     // input file is not encoded correctly
struct unexpected_character {}; // character is not known to scanner / misplaced

using token_variant =
    meta::variant<white_space_separator, new_line_indentation,
                  block_start_indentation, // later composite
                  block_end_indentation,   // later composite
                  comment_literal, colon_separator, comma_separator, semicolon_separator, square_bracket_open,
                  square_bracket_close, bracket_open, bracket_close, string_literal, number_literal, identifier_literal,
                  operator_literal, invalid_encoding, unexpected_character>;

struct token_data {
    text_range range;
    token_variant data;
};

struct tokenizer {
    struct config {
        column_t tab_stops; ///< columns per tabstop
    };
    tokenizer(config c)
        : config_m(c) {}

    auto scan_file(const file_t &file) -> meta::co_enumerator<token_data> {
        auto input = file_input_t(file);
        while (true) {
            input.collapse();
            const auto opt_chr = input.peek_char();
            if (!opt_chr) {
                if (!input.has_more_peek()) co_return;
                co_yield scan_invalid_encoding(input);
                continue;
            }
            const auto chr = opt_chr.value();
            if (chr.is_white_space()) co_yield scan_white_space(input);
            if (chr.is_line_separator()) co_yield scan_new_line(chr, input);
            //            if (chr.is_decimal_number()) co_yield
            //            scan_number_literal(input); switch (chr.v) { case '"':
            //            co_yield scan_string_literal(input); case '#': co_yield
            //            scan_comment(input); case ':': co_yield scan_char(input,
            //            token_type::ColonSeparator); case ',': co_yield
            //            scan_char(input, token_type::CommaSeparator); case ';':
            //            co_yield scan_char(input, token_type::SemicolonSeparator);
            //            case '[': co_yield scan_char(input,
            //            token_type::SquareBracketOpen); case ']': co_yield
            //            scan_char(input, token_type::SquareBracketClose); case '(':
            //            co_yield scan_char(input, token_type::BracketOpen); case
            //            ')': co_yield scan_char(input, token_type::BracketClose);
            //            }
            //            auto ident_token = scan_identifier(input);
            //            if (ident_token) co_yield ident_token.value();
            //            auto operator_token = scan_operator(input);
            //            if (operator_token) co_yield operator_token.value();

            co_yield scan_invalid(input);
        }
    }

    static auto scan_invalid_encoding(file_input_t &input) -> token_data {
        // invalid bytes were already skipped
        return {input.range(), invalid_encoding{}};
    }

    auto scan_white_space(file_input_t &input) const -> token_data {
        input.extend(config_m.tab_stops);
        input.extend_white_spaces(config_m.tab_stops);
        return {input.range(), white_space_separator{}};
    }

    auto scan_new_line(char_t chr, file_input_t &input) const -> token_data {
        input.skip();
        // skip 2nd char of newline pair
        if (chr == '\n' || chr == '\r') {
            input.peek_char().map([&](auto chr2) {
                if (chr2 != chr && (chr2 == '\n' || chr2 == '\r')) input.skip();
            });
        }
        input.next_line();
        input.extend_white_spaces(config_m.tab_stops);
        return {input.range(), new_line_indentation{}};
    }

    static auto scan_invalid(file_input_t &input) -> token_data {
        input.extend();
        return {input.range(), unexpected_character{}};
    }

private:
    config config_m;
};

} // namespace scanner
