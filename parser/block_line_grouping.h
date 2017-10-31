#pragma once
#include "scanner/tokenizer.h"

#include <vector>

namespace parser {

using view_t = strings::utf8_view;
using scan_token = scanner::token;
using token_line = std::vector<scan_token>;

using column_t = scanner::column_t;

struct block_literal {
    std::vector<token_line> lines;
};

namespace {

using optional_char = meta::optional<meta::packed<char>>;

struct state {
    optional_char indent_char{};

    inline auto get_indent_column(const scan_token &tok) -> column_t {
        const auto &range = tok.range;
        // TODO: extract indent char & verify it!
        // const auto &text = range.text;
        //        if (!text.is_empty()) {
        //            if (!indent_char) {
        //                auto opt_cp = view_t{text}.pull_code_point();
        //                if (!opt_cp || opt_cp.value().v > 255) {
        //                    // error
        //                }
        //                else {
        //                    indent_char = static_cast<char>(opt_cp.value().v);
        //                }
        //            }
        //            // TODO: verify indent char
        //        }
        return range.end_position.column;
    }
};

inline auto extract_line_tokens(token_line &line, meta::co_enumerator<scan_token> &input) {
    using namespace scanner;
    // TODO: add semicolon
    while (!input->one_of<new_line_indentation, block_start_indentation, block_end_indentation>()) {
        line.emplace_back(input.move());
        if (!input++) return;
    }
}

auto parse_block(meta::co_enumerator<scan_token> &input, column_t block_column, state &state_) -> block_literal;

inline auto parse_line(meta::co_enumerator<scan_token> &input, column_t parent_block_column, state &state_)
    -> token_line {
    auto line = token_line{};
    auto expect_end = false;
    while (true) {
        extract_line_tokens(line, input);
        if (!input) return line;

        while (true) {
            auto next_column = state_.get_indent_column(*input);
            if (input->one_of<scanner::new_line_indentation>()) {
                if (next_column < parent_block_column) {
                    if (expect_end) {
                        // TODO report missing end
                        // handling terminate line anyways
                    }
                    return line; // end of line in parent
                }
                if (next_column == parent_block_column && !expect_end) return line; // regular line break
                // next_column > parent_block_column => continuation
                while (true) {
                    if (!input++) return line;
                    extract_line_tokens(line, input);
                    if (!input) return line;

                    if (!input->one_of<scanner::new_line_indentation>()) break;
                    // auto continue_column = state_.get_indent_column(*input);
                    // if (continue_column >= next_column) continue;
                    // TODO report continuation error
                    // TODO handling: add lines to a block as well
                }
            }
            else if (input->one_of<scanner::block_end_indentation>()) {
                if (next_column < parent_block_column) {
                    if (expect_end) {
                        // TODO report missing end
                        // handling terminate line anyways
                    }
                    return line; // end of block in parent
                }
                if (next_column == parent_block_column) {
                    if (!expect_end) {
                        // TODO report unexpected end
                        // handling terminate line anyways
                    }
                    input++; // consume the end
                    return line;
                }
                // TODO report nested end
                // handling: ignored
                if (!input++) return line;
            }
            else if (input->one_of<scanner::block_start_indentation>()) {
                expect_end = true;
                if (next_column < parent_block_column) {
                    // TODO report missing end
                    // handling: add empty block and finish line
                    // line.emplace_back({input->range, block_literal{}});
                    return line;
                }
                if (next_column == parent_block_column) { // empty block
                    // line.emplace_back({input->range, block_literal{}});
                    if (!input++) return line;
                }
                else {
                    auto range = input->range;
                    auto block = parse_block(input, next_column, state_);
                    // line.emplace_back({std::move(range), std::move(block)});
                }
            }
            else
                break;
        }
    }
}

inline auto parse_block(meta::co_enumerator<scan_token> &input, column_t block_column, state &state_) -> block_literal {
    using namespace scanner;
    auto block = block_literal{};
    while (true) {
        while (true) {
            if (input->one_of<block_end_indentation>()) {
                auto indent = state_.get_indent_column(*input);
                if (indent < block_column) return block; // do not consume parent end block
                // TODO report misplaced end
                // handling: ignore it
                if (!input++) return block;
            }
            else if (input->one_of<block_start_indentation, new_line_indentation>()) {
                auto indent = state_.get_indent_column(*input);
                if (indent < block_column) return block; // line is not part of this block
                if (indent > block_column) {
                    // TODO: report indentation error
                    // handling: take the line into this block
                }
                if (!input++) return block;
            }
            else
                break;
        }
        auto line = parse_line(input, block_column, state_);
        block.lines.emplace_back(std::move(line));
        if (!input) break;
    }
    return block;
}

} // namespace

auto group_input(meta::co_enumerator<scan_token> input) -> block_literal {
    if (!input++) return {};
    auto state_ = state{};
    auto block_column = column_t{};
    if (input->one_of<scanner::new_line_indentation>()) {
        block_column = state_.get_indent_column(*input);
        if (!input++) return {};
    }
    auto block = parse_block(input, block_column, state_);
    if (input) {
        // TODO: report extra input
    }
    return block;
}

} // namespace parser
