#pragma once
#include "nesting/Token.h"

#include "meta/CoEnumerator.h"
#include "meta/Unreachable.h"

namespace nesting {

using View = strings::View;
using Column = text::Column;
using FilterToken = filter::Token;

/**
 * @brief block and line grouping parser is the 2nd parser step
 *
 * scans all the indentations and blocks
 *
 */
inline auto nestTokens(meta::CoEnumerator<FilterToken> input) -> BlockLiteral {
    // using OptionalChar = meta::Optional<meta::DefaultPacked<char>>;
    struct State {
        // OptionalChar indentChar{};

        auto getIndentColumn(const FilterToken& tok) -> Column {
            const auto& range = tok.visit([](auto& t) -> decltype(auto) { return t.range; });
            // TODO(arBmind): extract indent char & verify it!
            // const auto &text = range.text;
            //        if (!text.isEmpty()) {
            //            if (!indentChar) {
            //                auto optCp = View{text}.pullCodePoint();
            //                if (!optCp || optCp.value().v > 255) {
            //                    // error
            //                }
            //                else {
            //                    indentChar = static_cast<char>(optCp.value().v);
            //                }
            //            }
            //            // TODO(arBmind): verify indent char
            //        }
            return range.end.column;
        }
    };
    using Input = meta::CoEnumerator<FilterToken>;
    using BlockToken = nesting::Token;
    using BlockLine = nesting::TokenLine;

    if (!input++) return {};
    auto state_ = State{};

    auto translate = [](FilterToken&& tok) -> BlockToken {
        return std::move(tok).visit(
            [](filter::NewLineIndentation&&) { return meta::unreachable<BlockToken>(); },
            [](filter::BlockStartIndentation&&) { return meta::unreachable<BlockToken>(); },
            [](filter::BlockEndIndentation&&) { return meta::unreachable<BlockToken>(); },
            [](filter::SemicolonSeparator&&) { return meta::unreachable<BlockToken>(); },
            [](auto&& d) -> BlockToken { return std::move(d); });
    };

    auto extractLineTokens = [&](BlockLine& line, Input& input) {
        using namespace filter;
        while (!input->holds<NewLineIndentation, BlockStartIndentation, BlockEndIndentation, SemicolonSeparator>()) {
            line.emplace_back(translate(input.move()));
            if (!input++) return;
        }
    };

    auto parseLine = [&](Column parentBlockColumn, auto& parseBlock, auto& parseLine) -> BlockLine {
        using namespace filter;
        auto line = BlockLine{};
        auto expectEnd = false;
        while (true) {
            extractLineTokens(line, input);
            if (!input) return line;

            while (true) {
                if (input->holds<SemicolonSeparator>()) {
                    if (expectEnd) {
                        // TODO(arBmind): report error
                        // handling ignore
                    }
                    input++; // consume semicolon
                    return line; // semicolon terminates current line
                }
                else if (input->holds<NewLineIndentation>()) {
                    auto nextColumn = state_.getIndentColumn(*input);
                    if (nextColumn < parentBlockColumn) {
                        if (expectEnd) {
                            // TODO(arBmind): report missing end
                            // handling terminate line anyways
                        }
                        return line; // end of line in parent
                    }
                    if (nextColumn == parentBlockColumn && !expectEnd) return line; // regular line break
                    // nextColumn > parentBlockColumn => continuation
                    while (true) {
                        if (!input++) return line;
                        extractLineTokens(line, input);
                        if (!input) return line;

                        if (!input->holds<NewLineIndentation>()) break;
                        // auto continueColumn = state_.getIndentColumn(*input);
                        // if (continueColumn >= nextColumn) continue;
                        // TODO(arBmind): report continuation error
                        // TODO(arBmind): handling: add lines to a block as well
                    }
                }
                else if (input->holds<BlockEndIndentation>()) {
                    auto nextColumn = state_.getIndentColumn(*input);
                    if (nextColumn < parentBlockColumn) {
                        if (expectEnd) {
                            // TODO(arBmind): report missing end
                            // handling terminate line anyways
                        }
                        return line; // end of block in parent
                    }
                    if (nextColumn == parentBlockColumn) {
                        if (!expectEnd) {
                            // TODO(arBmind): report unexpected end
                            // handling terminate line anyways
                        }
                        input++; // consume the end
                        return line;
                    }
                    // TODO(arBmind): report nested end
                    // handling: ignored
                    if (!input++) return line;
                }
                else if (input->holds<BlockStartIndentation>()) {
                    auto nextColumn = state_.getIndentColumn(*input);
                    expectEnd = true;
                    if (nextColumn < parentBlockColumn) {
                        // TODO(arBmind): report missing end
                        // handling: add empty block and finish line
                        line.push_back(BlockLiteral{{}, input->get<BlockStartIndentation>().range});
                        return line;
                    }
                    if (nextColumn == parentBlockColumn) { // empty block
                        line.push_back(BlockLiteral{{}, input->get<BlockStartIndentation>().range});
                        if (!input++) return line;
                    }
                    else {
                        auto range = input->get<BlockStartIndentation>().range;
                        auto block = parseBlock(nextColumn, parseBlock, parseLine);
                        line.push_back(BlockLiteral{std::move(block), std::move(range)});
                    }
                }
                else
                    break;
            }
        }
    };

    auto parseBlock = [&](Column blockColumn, auto& parseBlock, auto& parseLine) -> BlockLiteralValue {
        using namespace filter;
        auto block = BlockLiteralValue{};
        while (true) {
            while (true) {
                if (input->holds<SemicolonSeparator>()) {
                    if (!input++) return block;
                }
                else if (input->holds<BlockEndIndentation>()) {
                    auto indent = state_.getIndentColumn(*input);
                    if (indent < blockColumn) return block; // do not consume parent end block
                    // TODO(arBmind): report misplaced end
                    // handling: ignore it
                    if (!input++) return block;
                }
                else if (input->holds<BlockStartIndentation, NewLineIndentation>()) {
                    auto indent = state_.getIndentColumn(*input);
                    if (indent < blockColumn) return block; // line is not part of this block
                    if (indent > blockColumn) {
                        // TODO(arBmind): report indentation error
                        // handling: take the line into this block
                    }
                    if (!input++) return block;
                }
                else
                    break;
            }
            auto line = parseLine(blockColumn, parseBlock, parseLine);
            block.lines.emplace_back(std::move(line));
            if (!input) break;
        }
        return block;
    };

    auto blockColumn = Column{};
    if (input->holds<filter::NewLineIndentation>()) {
        blockColumn = state_.getIndentColumn(*input);
        if (!input++) return {};
    }
    auto block = parseBlock(blockColumn, parseBlock, parseLine);
    if (input) {
        // TODO(arBmind): report extra input
    }
    return {block, {}};
}

} // namespace nesting
