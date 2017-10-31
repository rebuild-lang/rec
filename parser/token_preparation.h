#pragma once

#include "parser/prepared_token.h"
#include "scanner/tokenizer.h"

namespace parser {

using scan_token = scanner::token;
using scan_token_index = scanner::token_variant::index_t;

struct token_preparation {
    using prep_token = prepared::token;
    using text_range = prepared::text_range;

    static inline auto prepare(meta::co_enumerator<scan_token> input) -> meta::co_enumerator<prep_token> {
        using namespace prepared;
        while (true) {
            if (!input++) co_return;
            if (input->one_of<
                    scanner::comment_literal,
                    scanner::white_space_separator,
                    scanner::invalid_encoding,
                    scanner::unexpected_character>())
                continue; // skip initial values;
            break;
        }
        auto current = input.move();
        if (!current.one_of<scanner::new_line_indentation>()) {
            // ensure we always start with a new line
            co_yield prep_token{before_range(current), new_line_indentation{}};
        }
        auto lastYieldType = token_variant::index_of<new_line_indentation>();
        auto previous = translate(std::move(current));
        if (previous.one_of<identifier_literal, operator_literal>()) {
            mark_left_separator(previous);
        }
        while (input++) {
            auto previousOrSkippedType = current.data.index();
            current = input.move();

            if (previous.one_of<identifier_literal, operator_literal>()) {
                if (is_right_separator(current)) mark_right_separator(previous);
            }
            if (previous.one_of<colon_separator>()) {
                // this might be a block start, keep it in buffer
                while (current.one_of<scanner::white_space_separator, scanner::comment_literal>()) {
                    if (!input++) {
                        co_yield previous;
                        co_return;
                    }
                    current = input.move();
                }
            }
            if (current.one_of<
                    scanner::invalid_encoding,
                    scanner::unexpected_character,
                    scanner::comment_literal,
                    scanner::white_space_separator>()) {
                continue; // skip these tokens
            }
            bool left_separated = false;
            if (current.one_of<scanner::new_line_indentation>()) {
                if (previous.one_of<colon_separator>()) {
                    // if (lastYieldType.one_of<new_line_indentation, block_start_indentation>()) {
                    // not allowed
                    //}
                    // we do not merge the range here, because there might be skipped comments between
                    previous = prep_token{std::move(current.range), block_start_indentation{}};
                    continue; // [':' + '\n'] => block start
                }
                if (previous.one_of<new_line_indentation>()) {
                    // TODO: collapse range
                    continue; // skip second newline
                }
            }
            else if (current.one_of<scanner::operator_literal>()) {
                if (is_left_separator(previousOrSkippedType)) left_separated = true;
            }
            else if (current.one_of<scanner::identifier_literal>()) {
                if (previous.one_of<new_line_indentation, block_start_indentation>() &&
                    current.range.text.content_equals(view_t{"end"})) {
                    if (!previous.one_of<new_line_indentation>()) {
                        lastYieldType = previous.data.index();
                        co_yield previous;
                    }
                    previous = prep_token{previous.range, block_end_indentation{}};
                    continue; // ['\n' + "end"] => block end
                }
                if (is_left_separator(previousOrSkippedType)) left_separated = true;
            }
            lastYieldType = previous.data.index();
            co_yield previous;
            previous = translate(std::move(current));
            if (left_separated) mark_left_separator(previous);
        }
        if (previous.one_of<identifier_literal, operator_literal>()) mark_right_separator(previous);
        if (!previous.one_of<new_line_indentation>()) co_yield previous;
    }

private:
    constexpr static inline bool is_right_separator(scan_token_index index) {
        using namespace scanner;
        return index.holds< //
            white_space_separator, // tok[\s]
            new_line_indentation, // tok[\n]
            comment_literal, // tok#* *#
            colon_separator, // tok:
            comma_separator, // tok,
            semicolon_separator, // tok;
            square_bracket_close, // tok]
            bracket_close // tok)
            >();
    }
    static inline bool is_right_separator(const scan_token &t) { return is_right_separator(t.data.index()); }

    constexpr static inline bool is_left_separator(scan_token_index index) {
        using namespace scanner;
        return index.holds< //
            white_space_separator, // [\s]tok
            new_line_indentation, // [\n]tok
            comment_literal, // #* *#tok
            colon_separator, // :tok
            comma_separator, // ,tok
            semicolon_separator, // ;tok
            square_bracket_open, // [tok
            bracket_open // (tok
            >();
    }

    static inline auto before_range(const scan_token &tok) -> text_range {
        return {tok.range.file, {}, {}, tok.range.begin_position};
    }

    static inline void mark_right_separator(prep_token &tok) {
        using namespace prepared;
        tok.data.visit_some(
            [](identifier_literal &l) { l.right_separated = true; },
            [](operator_literal &o) { o.right_separated = true; });
    }

    static inline void mark_left_separator(prep_token &tok) {
        using namespace prepared;
        tok.data.visit_some(
            [](identifier_literal &l) { l.left_separated = true; },
            [](operator_literal &o) { o.left_separated = true; });
    }

    static inline prep_token translate(scan_token &&tok) {
        using namespace prepared;
        return std::move(tok.data).visit(
            [](scanner::white_space_separator &&) { return prep_token{}; },
            [](scanner::comment_literal &&) { return prep_token{}; },
            [](scanner::invalid_encoding &&) { return prep_token{}; },
            [](scanner::unexpected_character &&) { return prep_token{}; },
            [&](scanner::identifier_literal &&) {
                return prep_token{std::move(tok.range), identifier_literal{}};
            },
            [&](scanner::operator_literal &&) {
                return prep_token{std::move(tok.range), operator_literal{}};
            },
            [&](auto &&d) {
                return prep_token{std::move(tok.range), std::move(d)};
            });
    }
};

} // namespace parser