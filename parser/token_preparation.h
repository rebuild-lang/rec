#pragma once

#include "scanner/tokenizer.h"

namespace parser {

using token_data = scanner::token_data;
using text_range = scanner::text_range;

template<class T>
using optional = meta::optional<T>;

bool is_skipable(const token_data &tok) {
    return meta::holds_one_of<scanner::comment_literal, scanner::white_space_separator, scanner::invalid_encoding,
                              scanner::unexpected_character>(tok.data);
}
bool is_newline(const token_data &tok) { return std::holds_alternative<scanner::new_line_indentation>(tok.data); }
auto before_range(const token_data &tok) -> text_range { return {tok.range.file, {}, {}, tok.range.begin_position}; }

auto prepare_tokens(meta::co_enumerator<token_data> input) -> meta::co_enumerator<token_data> {
    token_data tok;
    while (input) {
        tok = *input;
        if (!is_skipable(tok)) break;
        input++;
    }

    if (!is_newline(tok)) {
        // ensure we always start with a new line
        co_yield token_data{before_range(tok), scanner::new_line_indentation{}};
    }

    //        input().map([&](token_data tok) -> optional<token_data> {
    //            return {};
    //        });
}

} // namespace parser
