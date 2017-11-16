#pragma once

#include "meta/co_enumerator.h"

#include "file_input.h"
#include "number_scanner.h"
#include "token.h"

namespace scanner {

struct tokenizer {
    struct config {
        column_t tab_stops; ///< columns per tabstop
    };
    tokenizer(config c)
        : config_m(c) {}

    auto scan_file(const file_t &file) -> meta::co_enumerator<token> {
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
            if (chr.is_decimal_number()) co_yield scan_number_literal(chr, input);
            // switch (chr.v) {
            // case '"': co_yield scan_string_literal(input);
            // case '#': co_yield scan_comment(input);
            // case ':': co_yield scan_char(input, token_type::ColonSeparator);
            // case ',': co_yield scan_char(input, token_type::CommaSeparator);
            // case ';': co_yield scan_char(input, token_type::SemicolonSeparator);
            // case '[': co_yield scan_char(input, token_type::SquareBracketOpen);
            // case ']': co_yield scan_char(input, token_type::SquareBracketClose);
            // case '(': co_yield scan_char(input, token_type::BracketOpen);
            // case ')': co_yield scan_char(input, token_type::BracketClose);
            //}
            // auto ident_token = scan_identifier(input);
            // if (ident_token) co_yield ident_token.value();
            // auto operator_token = scan_operator(input);
            // if (operator_token) co_yield operator_token.value();

            co_yield scan_invalid(input);
        }
    }

private:
    static auto scan_number_literal(char_t chr, file_input_t &input) -> token {
        return number_scanner::scan(chr, input);
    }

    static auto scan_invalid_encoding(file_input_t &input) -> token {
        // invalid bytes were already skipped
        return {input.range(), invalid_encoding{}};
    }

    auto scan_white_space(file_input_t &input) const -> token {
        input.extend(config_m.tab_stops);
        input.extend_white_spaces(config_m.tab_stops);
        return {input.range(), white_space_separator{}};
    }

    auto scan_new_line(char_t chr, file_input_t &input) const -> token {
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

    static auto scan_invalid(file_input_t &input) -> token {
        input.extend();
        return {input.range(), unexpected_character{}};
    }

private:
    config config_m;
};

} // namespace scanner
