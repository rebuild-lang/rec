#pragma once

#include "meta/variant.h"
#include "meta/optional.h"
#include "strings/utf8_string.h"
#include "strings/utf8_view.h"
#include "strings/code_point.h"

#include <deque>

namespace scanner {

using string_t = strings::utf8_string;
using view_t = strings::utf8_view;
using char_t = strings::code_point_t;
using opt_char_t = strings::optional_code_point_t;

template<class... Ts> using variant = meta::variant<Ts...>;
template<class T> using optional = meta::optional<T>;

struct file_t {
    string_t filename;
    string_t content;
};

struct line_t {
    uint32_t v;

    constexpr line_t() : v(1) {} // start with line 1

    constexpr line_t(const line_t&) = default;
    constexpr line_t& operator=(const line_t&) = default;
    constexpr line_t(line_t&&) = default;
    constexpr line_t& operator=(line_t&&) = default;

    constexpr line_t& operator++() noexcept { v++; return *this; }
};

struct column_t {
    uint32_t v;

    constexpr column_t() : v(1) {} // start with column 1
    constexpr explicit column_t(uint32_t x) : v(x) {}

    constexpr column_t(const column_t&) = default;
    constexpr column_t& operator=(const column_t&) = default;
    constexpr column_t(column_t&&) = default;
    constexpr column_t& operator=(column_t&&) = default;

    constexpr column_t& operator++() noexcept { v++; return *this; }
};

struct position_t {
    line_t line;
    column_t column;

    constexpr void next_column() noexcept { ++column; }
    constexpr void next_tabstop(column_t tabstop) noexcept {
        column.v += tabstop.v - (column.v % tabstop.v);
    }
    constexpr void next_line() noexcept { ++line; column = {}; }
};

struct text_range {
    const file_t* file;
    view_t text;
    position_t begin_position;
    position_t end_position;
};

struct file_input_t {
    using value_ptr = const string_t::value_type*;

    file_input_t(const file_t& file)
        : file_ptr(&file)
        , begin_ptr(file.content.data())
        , current_ptr(file.content.data())
        , peek_ptr(file.content.data())
        , begin_position({})
        , current_position({})
    {}

    auto range() const {
        return text_range{
            file_ptr,
            view_t(begin_ptr, current_ptr),
            begin_position, current_position
        };
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

    template<size_t N = 0>
    opt_char_t peek_char() {
        if (peek_buffer.size() < N) {
            if (!fill_peek(N)) return {};
        }
        return peek_buffer[N];
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
        }));
    }

    void collapse_white_spaces(column_t tabstop) {
        extend_white_spaces(tabstop);
        collapse();
    }

    void next_line() {
        current_position.next_line();
    }

private:
    value_ptr end_ptr() const { return file_ptr->content.data() + file_ptr->content.byte_count().v; }
    bool fill_peek(size_t count);

private:
    const file_t* file_ptr;
    value_ptr begin_ptr;
    value_ptr current_ptr;
    value_ptr peek_ptr;
    position_t begin_position;
    position_t current_position;
    std::deque<char_t> peek_buffer;
};

enum class token_type {
    WhiteSpaceSeparator,
    NewLineIndentation,
    BlockStartIndentation, // used later
    BlockEndIndentation, // used later
    Comment,
    ColonSeparator,
    CommaSeparator,
    SemicolonSeparator,
    SquareBracketOpen,
    SquareBracketClose,
    BracketOpen,
    BracketClose,
    StringLiteral,
    NumberLiteral,
    IdentifierLiteral,
    OperatorLiteral,
};
struct number_literal {
};
enum class scan_error {
    InvalidEncoding, // input file is not encoded correctly
    UnexpectedCharacter, // character is not known to scanner / misplaced
};
struct token_data {
    text_range range;
    variant<token_type, number_literal, scan_error> data;
};

struct tokenizer {
    struct config {
        column_t tab_stops; ///< columns per tabstop
    };
    tokenizer(config c) : config_m(c) {}

    auto scan_file(const file_t &file) {
        auto input = file_input_t(file);
        return [=]() mutable -> optional<token_data> {
            input.collapse();
            const auto opt_chr = input.peek_char();
            if (!opt_chr) return {};
            const auto chr = opt_chr.value();
            if (chr.is_white_space()) return scan_white_space(input);
            if (chr.is_line_separator()) return scan_new_line(chr, input);
//            if (chr.is_decimal_number()) return scan_number_literal(input);
//            switch (chr.v) {
//            case '"': return scan_string_literal(input);
//            case '#': return scan_comment(input);
//            case ':': return scan_char(input, token_type::ColonSeparator);
//            case ',': return scan_char(input, token_type::CommaSeparator);
//            case ';': return scan_char(input, token_type::SemicolonSeparator);
//            case '[': return scan_char(input, token_type::SquareBracketOpen);
//            case ']': return scan_char(input, token_type::SquareBracketClose);
//            case '(': return scan_char(input, token_type::BracketOpen);
//            case ')': return scan_char(input, token_type::BracketClose);
//            }
//            auto ident_token = scan_identifier(input);
//            if (ident_token) return ident_token.value();
//            auto operator_token = scan_operator(input);
//            if (operator_token) return operator_token.value();

            return scan_invalid(input);
        };
    }

    auto scan_white_space(file_input_t& input) const -> token_data {
        input.extend(config_m.tab_stops);
        input.extend_white_spaces(config_m.tab_stops);
        return {input.range(), token_type::WhiteSpaceSeparator};
    }

    auto scan_new_line(char_t chr, file_input_t& input) const -> token_data {
        input.skip();
        // skip 2nd char of newline pair
        if (chr == '\n' || chr == '\r') {
            input.peek_char().map([&](auto chr2) {
               if (chr2 != chr && (chr2 == '\n' || chr2 == '\r'))
                   input.skip();
            });
        }
        input.next_line();
        input.extend_white_spaces(config_m.tab_stops);
        return {input.range(), token_type::NewLineIndentation};
    }

    static auto scan_invalid(file_input_t& input) -> token_data {
        input.extend();
        return {input.range(), scan_error::UnexpectedCharacter};
    }

private:
    config config_m;
};

} // namespace scanner
