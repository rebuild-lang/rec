#pragma once

#include "scanner/tokenizer.h"

namespace parser {

using token_data = scanner::token_data;
using text_range = scanner::text_range;

template<class T>
using optional = meta::optional<T>;

namespace {

bool is_skipable(const token_data &tok) {
    using namespace scanner;
    return tok.one_of<comment_literal, white_space_separator, invalid_encoding, unexpected_character>();
}
bool is_newline(const token_data &tok) {
    using namespace scanner;
    return tok.one_of<new_line_indentation>();
}

bool is_right_separator(int index) {
    using namespace scanner;
    return meta::holds_one_of<token_variant,         //
                              white_space_separator, // tok[\s]
                              new_line_indentation,  // tok[\n]
                              comment_literal,       // tok#* *#
                              colon_separator,       // tok:
                              comma_separator,       // tok,
                              semicolon_separator,   // tok;
                              square_bracket_close,  // tok]
                              bracket_close,         // tok)
                              block_end_indentation  // tok[\n]end
                              >(index);
}
bool is_right_separator(const token_data &t) { return is_right_separator(t.data.index()); }

bool is_left_separator(int index) {
    using namespace scanner;
    return meta::holds_one_of<token_variant,          //
                              white_space_separator,  // [\s]tok
                              new_line_indentation,   // [\n]tok
                              comment_literal,        // #* *#tok
                              colon_separator,        // :tok
                              comma_separator,        // ,tok
                              semicolon_separator,    // ;tok
                              square_bracket_open,    // [tok
                              bracket_open,           // (tok
                              block_start_indentation // :[\n]tok
                              >(index);
}
bool is_left_separator(const token_data &t) { return is_left_separator(t.data.index()); }

auto before_range(const token_data &tok) -> text_range { return {tok.range.file, {}, {}, tok.range.begin_position}; }

void mark_right_separator(token_data &tok) {
    using namespace scanner;
    std::get<identifier_literal>(tok.data).right_separated = true;
}

void mark_left_separator(token_data &tok) {
    using namespace scanner;
    std::get<identifier_literal>(tok.data).left_separated = true;
}

} // namespace

auto prepare_tokens(meta::co_enumerator<token_data> input) -> meta::co_enumerator<token_data> {
    using namespace scanner;
    while (true) {
        if (!input++) co_return;
        if (input->one_of<comment_literal, white_space_separator, invalid_encoding, unexpected_character>())
            continue; // skip initial values;
        break;
    }
    auto current = input.move();
    if (!current.one_of<new_line_indentation>()) {
        // ensure we always start with a new line
        co_yield token_data{before_range(current), new_line_indentation{}};
    }
    auto lastYieldType = meta::type_index<token_variant, new_line_indentation>;
    if (current.one_of<identifier_literal, operator_literal>()) {
        mark_left_separator(current);
    }
    auto previous = std::move(current);
    while (!input++) {
        auto previousOrSkippedType = current.data.index();
        current = input.move();

        if (previous.one_of<identifier_literal, operator_literal>()) {
            if (is_right_separator(current)) mark_right_separator(previous);
        }
        if (previous.one_of<colon_separator>()) {
            // this might be a block start, keep it in buffer
            while (current.one_of<white_space_separator, comment_literal>()) {
                if (!input++) {
                    co_yield previous;
                    co_return;
                }
                current = input.move();
            }
        }
        if (current.one_of<invalid_encoding, unexpected_character, comment_literal, white_space_separator>()) {
            continue; // skip these tokens
        }
        if (current.one_of<new_line_indentation>()) {
            if (previous.one_of<colon_separator>()) {
                // if (lastYieldType.one_of<new_line_indentation, block_start_indentation>()) {
                // not allowed
                //}
                // we do not merge the range here, because there might be skipped comments between
                previous = token_data{current.range, block_start_indentation{}};
                continue; // [':' + '\n'] => block start
            }
            if (previous.one_of<new_line_indentation>()) {
                continue; // skip second newline
            }
        }
        else if (current.one_of<operator_literal>()) {
            if (is_left_separator(previousOrSkippedType)) mark_left_separator(current);
        }
        else if (current.one_of<identifier_literal>()) {
            if (previous.one_of<new_line_indentation>() && current.range.text.content_equals(view_t{"end"})) {
                previous = token_data{previous.range, block_end_indentation{}};
                continue; // ['\n' + "end"] => block end
            }
            if (is_left_separator(previousOrSkippedType)) mark_left_separator(current);
        }
        lastYieldType = previous.data.index();
        co_yield previous;
        previous = std::move(current);
    }
    if (previous.one_of<identifier_literal, operator_literal>()) {
        if (is_right_separator(current)) mark_right_separator(previous);
    }
    if (!previous.one_of<new_line_indentation>()) co_yield previous;
}

} // namespace parser
