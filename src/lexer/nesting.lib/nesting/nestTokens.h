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
    using Input = meta::CoEnumerator<FilterToken>;
    using BlockToken = nesting::Token;
    using BlockLine = nesting::TokenLine;

    //    struct State {
    //        // OptionalChar indentChar{};

    //        auto getIndentColumn(const text::InputPositionData& data) -> Column {
    //            // TODO(arBmind): extract indent char & verify it!
    //            // const auto &text = range.text;
    //            //        if (!text.isEmpty()) {
    //            //            if (!indentChar) {
    //            //                auto optCp = View{text}.pullCodePoint();
    //            //                if (!optCp || optCp.value().v > 255) {
    //            //                    // error
    //            //                }
    //            //                else {
    //            //                    indentChar = static_cast<char>(optCp.value().v);
    //            //                }
    //            //            }
    //            //            // TODO(arBmind): verify indent char
    //            //        }
    //            return data.position.column;
    //        }
    //    };
    //    auto state_ = State{};

    //    auto extractLineTokens = [&](BlockLine& line, Input& input) {
    //        using namespace filter;
    //        while (true) {
    //            auto handled = input.move().visit(
    //                [](filter::NewLineIndentation&&) { return false; },
    //                [](filter::BlockStartIndentation&&) { return false; },
    //                [](filter::BlockEndIndentation&&) { return false; },
    //                [](filter::SemicolonSeparator&&) { return false; },
    //                [&](auto&& d) {
    //                    line.emplace_back(std::move(d));
    //                    return true;
    //                });
    //            if (!handled) return;
    //            if (!input++) return;
    //        }
    //    };

    //    enum class LineStep {
    //        Continue,
    //        Return,
    //        Break,
    //    };
    //    auto parseLine = [&](Column parentBlockColumn, auto& parseBlock, auto& parseLine) -> BlockLine {
    //        using namespace filter;
    //        auto line = BlockLine{};
    //        auto expectEnd = false;

    //        auto handleSemicolon = [&]() -> LineStep {
    //            if (expectEnd) {
    //                // TODO(arBmind): report error
    //                // handling: ignore
    //            }
    //            input++; // consume semicolon
    //            return LineStep::Return; // semicolon terminates current line
    //        };
    //        auto handleNewLine = [&](const TextRange& range) -> LineStep {
    //            auto nextColumn = state_.getIndentColumn(range);
    //            if (nextColumn < parentBlockColumn) {
    //                if (expectEnd) {
    //                    // TODO(arBmind): report missing end
    //                    // handling terminate line anyways
    //                }
    //                return LineStep::Return; // end of line in parent
    //            }
    //            if (nextColumn == parentBlockColumn && !expectEnd) return LineStep::Return; // regular line break
    //            // nextColumn > parentBlockColumn => continuation
    //            while (true) {
    //                if (!input++) return LineStep::Return;
    //                extractLineTokens(line, input);
    //                if (!input) return LineStep::Return;

    //                if (!input->holds<NewLineIndentation>()) break;
    //                // auto continueColumn = state_.getIndentColumn(*input);
    //                // if (continueColumn >= nextColumn) continue;
    //                // TODO(arBmind): report continuation error
    //                // TODO(arBmind): handling: add lines to a block as well
    //            }
    //            return LineStep::Continue;
    //        };
    //        auto handleBlockEnd = [&](const TextRange& range) -> LineStep {
    //            auto endColumn = range.begin.column;
    //            if (endColumn < parentBlockColumn) {
    //                if (expectEnd) {
    //                    // TODO(arBmind): report missing end
    //                    // handling terminate line anyways
    //                }
    //                return LineStep::Return; // end of block in parent
    //            }
    //            if (endColumn == parentBlockColumn) {
    //                if (!expectEnd) {
    //                    // TODO(arBmind): report unexpected end
    //                    // handling terminate line anyways
    //                }
    //                input++; // consume the end
    //                return LineStep::Return;
    //            }
    //            // TODO(arBmind): report nested end
    //            // handling: ignored
    //            if (!input++) return LineStep::Return;
    //            return LineStep::Continue;
    //        };
    //        auto handleBlockStart = [&](TextRange range) -> LineStep {
    //            auto nextColumn = state_.getIndentColumn(range);
    //            expectEnd = true;
    //            if (nextColumn < parentBlockColumn) {
    //                // TODO(arBmind): report missing end
    //                // handling: add empty block and finish line
    //                // line.push_back(BlockLiteral{{}, range});
    //                return LineStep::Return;
    //            }
    //            if (nextColumn == parentBlockColumn) { // empty block
    //                // line.push_back(BlockLiteral{{}, range});
    //                if (!input++) return LineStep::Return;
    //                return LineStep::Continue;
    //            }
    //            auto block = parseBlock(nextColumn, parseBlock, parseLine);
    //            // line.push_back(BlockLiteral{std::move(block), std::move(range)});
    //            return LineStep::Continue;
    //        };

    //        while (true) {
    //            extractLineTokens(line, input);
    //            if (!input) return line;

    //            while (true) {
    //                auto step = input->visit(
    //                    [&](const SemicolonSeparator&) { return handleSemicolon(); },
    //                    /*
    //                        [&](const NewLineIndentation& newLine) { return handleNewLine(newLine.range); },
    //                        [&](const BlockEndIndentation& blockEnd) { return handleBlockEnd(blockEnd.range); },
    //                        [&](const BlockStartIndentation& blockStart) { return handleBlockStart(blockStart.range);
    //                        },
    //                        */
    //                    [](const auto& v) { return LineStep::Break; });
    //                if (step == LineStep::Return) return line;
    //                if (step == LineStep::Break) break;
    //            }
    //        }
    //    };

    //    enum class BlockStep {
    //        Continue,
    //        Return,
    //        Break,
    //    };
    //    auto parseBlock = [&](Column blockColumn, auto& parseBlock, auto& parseLine) -> BlockLiteralValue {
    //        using namespace filter;
    //        auto block = BlockLiteralValue{};

    //        auto handleSemicolon = [&]() -> BlockStep {
    //            if (!input++) return BlockStep::Return;
    //            return BlockStep::Continue;
    //        };
    //        auto handleBlockEnd = [&](const TextRange& range) -> BlockStep {
    //            auto endColumn = range.begin.column;
    //            if (endColumn < blockColumn) return BlockStep::Return; // do not consume parent end block
    //            // TODO(arBmind): report misplaced end
    //            // handling: ignore it
    //            if (!input++) return BlockStep::Return;
    //            return BlockStep::Continue;
    //        };
    //        auto handleNewLine = [&](const TextRange& range) -> BlockStep {
    //            auto nextColumn = state_.getIndentColumn(range);
    //            if (nextColumn < blockColumn) return BlockStep::Return; // line is not part of this block
    //            if (nextColumn > blockColumn) {
    //                // TODO(arBmind): report indentation error
    //                // handling: take the line into this block
    //            }
    //            if (!input++) return BlockStep::Return;
    //            return BlockStep::Continue;
    //        };

    //        while (true) {
    //            while (true) {
    //                /* auto step = input->visit(
    //                    [&](const SemicolonSeparator&) { return handleSemicolon(); },
    //                    [&](const BlockEndIndentation& blockEnd) { return handleBlockEnd(blockEnd.range); },
    //                    [&](const BlockStartIndentation& blockStart) { return handleNewLine(blockStart.range); },
    //                    [&](const NewLineIndentation& newLine) { return handleNewLine(newLine.range); },
    //                    [](const auto&) { return BlockStep::Break; });

    //                if (step == BlockStep::Return) return block;
    //                if (step == BlockStep::Break) break;
    //                */
    //            }
    //            auto line = parseLine(blockColumn, parseBlock, parseLine);
    //            block.lines.emplace_back(std::move(line));
    //            if (!input) break;
    //        }
    //        return block;
    //    };

    //    if (!input++) return {};
    //    auto blockColumn = Column{};
    //    if (input->holds<filter::NewLineIndentation>()) {
    //        // blockColumn = state_.getIndentColumn(input->get<filter::NewLineIndentation>().range);
    //        if (!input++) return {};
    //    }
    //    auto block = parseBlock(blockColumn, parseBlock, parseLine);
    //    if (input) {
    //        // TODO(arBmind): report extra input
    //    }
    //    return {{}, {}, block};
    return {};
}

} // namespace nesting
