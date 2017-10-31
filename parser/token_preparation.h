#pragma once

#include "scanner/tokenizer.h"

namespace parser {

using token = scanner::token;
using token_index = scanner::token_variant::index_t;
using text_range = scanner::text_range;

template<class T>
using optional = meta::optional<T>;

namespace {

constexpr bool is_right_separator(token_index index) {
    using namespace scanner;
    return index.holds< //
        white_space_separator, // tok[\s]
        new_line_indentation, // tok[\n]
        comment_literal, // tok#* *#
        colon_separator, // tok:
        comma_separator, // tok,
        semicolon_separator, // tok;
        square_bracket_close, // tok]
        bracket_close, // tok)
        block_end_indentation // tok[\n]end
        >();
}
bool is_right_separator(const token &t) { return is_right_separator(t.data.index()); }

constexpr bool is_left_separator(token_index index) {
    using namespace scanner;
    return index.holds< //
        white_space_separator, // [\s]tok
        new_line_indentation, // [\n]tok
        comment_literal, // #* *#tok
        colon_separator, // :tok
        comma_separator, // ,tok
        semicolon_separator, // ;tok
        square_bracket_open, // [tok
        bracket_open, // (tok
        block_start_indentation // :[\n]tok
        >();
}

auto before_range(const token &tok) -> text_range { return {tok.range.file, {}, {}, tok.range.begin_position}; }

void mark_right_separator(token &tok) {
    using namespace scanner;
    tok.data.visit_some(
        [](identifier_literal &l) { l.right_separated = true; }, [](operator_literal &o) { o.right_separated = true; });
}

void mark_left_separator(token &tok) {
    using namespace scanner;
    tok.data.visit_some(
        [](identifier_literal &l) { l.left_separated = true; }, [](operator_literal &o) { o.left_separated = true; });
}

} // namespace

auto prepare_tokens(meta::co_enumerator<token> input) -> meta::co_enumerator<token> {
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
        co_yield token{before_range(current), new_line_indentation{}};
    }
    auto lastYieldType = token_variant::index_of<new_line_indentation>();
    if (current.one_of<identifier_literal, operator_literal>()) {
        mark_left_separator(current);
    }
    auto previous = std::move(current);
    while (input++) {
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
                previous = token{current.range, block_start_indentation{}};
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
            if (previous.one_of<new_line_indentation, block_start_indentation>() &&
                current.data.get<identifier_literal>().content == view_t{"end"}) {
                if (!previous.one_of<new_line_indentation>()) {
                    lastYieldType = previous.data.index();
                    co_yield previous;
                }
                previous = token{previous.range, block_end_indentation{}};
                continue; // ['\n' + "end"] => block end
            }
            if (is_left_separator(previousOrSkippedType)) mark_left_separator(current);
        }
        lastYieldType = previous.data.index();
        co_yield previous;
        previous = std::move(current);
    }
    if (previous.one_of<identifier_literal, operator_literal>()) mark_right_separator(previous);
    if (!previous.one_of<new_line_indentation>()) co_yield previous;
}

} // namespace parser
