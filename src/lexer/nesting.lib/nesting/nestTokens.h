#pragma once
#include "nesting/Token.h"

#include "meta/CoEnumerator.h"
#include "meta/Unreachable.h"

namespace nesting {

using strings::View;
using text::Column;
using FilterTokenLine = filter::TokenLine;

/**
 * @brief block and line grouping parser is the 2nd parser step
 *
 * scans all the indentations and blocks
 *
 */
inline auto nestTokens(meta::CoEnumerator<FilterTokenLine> input) -> BlockLiteral {
    // using OptionalChar = meta::Optional<meta::DefaultPacked<char>>;
    using Input = meta::CoEnumerator<FilterTokenLine>;
    using BlockToken = nesting::Token;
    using BlockLine = nesting::BlockLine;

    auto extractLineTokens = [&](BlockLine& blockLine, FilterTokenLine&& line) {
        using namespace filter;
        for (auto& tok : line.tokens) {
            blockLine.tokens.push_back( //
                std::move(tok).visit([](auto&& d) -> BlockToken { return std::move(d); }));
        }
        for (auto& ins : line.insignificants) {
            blockLine.insignificants.push_back(std::move(ins).visit(
                [](BlockEndIdentifier&& b) -> Insignificant { return UnexpectedBlockEnd{b}; },
                [](auto&& d) -> Insignificant { return std::move(d); }));
        }
    };
    auto extractLineEndTokens = [&](BlockLine& blockLine, FilterTokenLine&& line) {
        using namespace filter;
        for (auto& tok : line.tokens) {
            blockLine.tokens.push_back( //
                std::move(tok).visit([](auto&& d) -> BlockToken { return std::move(d); }));
        }
        for (auto& ins : line.insignificants) {
            blockLine.insignificants.push_back(
                std::move(ins).visit([](auto&& d) -> Insignificant { return std::move(d); }));
        }
    };

    enum class LineType { WithEnd, BlockStart, BlockStartLeave, Standalone, LeaveBlock };
    struct ParseLineResult {
        LineType type;
        BlockLine line;
        text::Position position;
    };
    auto parseLineBlocks = [&](BlockLine& line,
                               Column parentBaseColumn,
                               Column parentBlockColumn,
                               auto& parseBlock,
                               auto& parseLine) -> ParseLineResult {
        auto lineIndent = parentBlockColumn;
        if (input->startsOnNewLine()) lineIndent = input->newLine().value.indentColumn;
        auto startColonPosition = input->blockStartColon().position;

        extractLineTokens(line, input.move());
        input++;
        if (!input) {
            line.tokens.push_back(BlockLiteral{{{}, startColonPosition}, {}});
            line.insignificants.push_back(MissingBlockEnd{{{}, startColonPosition}});
            return {LineType::LeaveBlock, std::move(line), {}}; // no more lines
        }
        if (!input->startsOnNewLine()) {
            line.tokens.push_back(BlockLiteral{{{}, startColonPosition}, {}});
            line.insignificants.push_back(MissingBlockEnd{{{}, startColonPosition}});
            return {LineType::Standalone, std::move(line), {}}; // second line on same line
        }

        auto blockNewLine = input->newLine();
        auto blockIndent = input->newLine().value.indentColumn;
        if (blockIndent <= lineIndent) {
            auto position = text::Position{blockNewLine.position.line, blockIndent};
            line.tokens.push_back(BlockLiteral{{{}, position}, {}});
            if (blockIndent <= parentBaseColumn) {
                return {LineType::BlockStartLeave, std::move(line), position}; // errornous empty block
            }
            return {LineType::BlockStart, std::move(line), position};
        }

        auto block = parseBlock(parentBlockColumn, blockIndent, parseBlock, parseLine);
        line.tokens.push_back(BlockLiteral{{}, std::move(block)});

        // process line after block
        if (!input) return {LineType::LeaveBlock, std::move(line), {}}; // no more input - TODO: handle error?
        if (!input->startsOnNewLine())
            return {LineType::WithEnd, std::move(line), {}}; // end not on new line - seems impossible

        auto endNewLine = input->newLine();
        auto endIndent = endNewLine.value.indentColumn;
        if (endIndent <= parentBaseColumn) {
            auto data = text::InputPositionData{{}, text::Position{endNewLine.position.line, endIndent}};
            line.insignificants.push_back(MissingBlockEnd{data});
            return {LineType::LeaveBlock, std::move(line), {}}; // missing end
        }
        if (endIndent > lineIndent) {
            // TODO: handle wrong indentation
        }
        // assert(endIndent == parentBlockColumn);
        // line is part of current line
        if (input->isBlockEnd()) {
            if (input->tokens.empty() && !input->isBlockStart()) {
                extractLineEndTokens(line, input.move());
                input++;
                return {LineType::WithEnd, std::move(line), {}}; // regular block end
            }
            // TODO: add UnexpectedTokens error
        }

        return {LineType::BlockStart, std::move(line), startColonPosition};
    };

    auto parseLine =
        [&](Column baseColumn, Column parentBlockColumn, auto& parseBlock, auto& parseLine) -> ParseLineResult {
        auto line = BlockLine{};

        if (input->isBlockStart()) return parseLineBlocks(line, baseColumn, parentBlockColumn, parseBlock, parseLine);

        auto lineIndent = parentBlockColumn;
        if (input->startsOnNewLine()) {
            lineIndent = input->newLine().value.indentColumn;
            // assert(lineIndent > baseColumn);
            if (lineIndent < parentBlockColumn) {
                line.insignificants.push_back(UnexpectedIndent{input->newLine()});
            }
        }
        extractLineTokens(line, input.move());

        input++;
        if (!input) return {LineType::LeaveBlock, std::move(line), {}}; // no further lines
        if (!input->startsOnNewLine()) return {LineType::Standalone, std::move(line), {}}; // second line on same line

        auto continueIndent = input->newLine().value.indentColumn;
        if (continueIndent <= lineIndent) {
            if (continueIndent <= baseColumn) return {LineType::LeaveBlock, std::move(line), {}}; // leave block
            return {LineType::Standalone, std::move(line), {}}; // no line continuation
        }

        while (true) {
            if (input->isBlockStart())
                return parseLineBlocks(line, baseColumn, parentBlockColumn, parseBlock, parseLine);

            extractLineTokens(line, input.move());
            input++;
            if (!input) return {LineType::LeaveBlock, std::move(line), {}}; // no further lines
            if (!input->startsOnNewLine())
                return {LineType::Standalone, std::move(line), {}}; // second line on same line

            auto nextIndent = input->newLine().value.indentColumn;
            if (nextIndent < continueIndent) {
                if (nextIndent <= baseColumn) {
                    return {LineType::LeaveBlock, std::move(line), {}}; // leave block
                }
                // TODO: handle mixed indentation
                return {LineType::Standalone, std::move(line), {}}; // no further continuation
            }
        }
    };

    static auto moveAppend = [](auto& dst, auto&& src) {
        dst.insert(dst.end(), std::make_move_iterator(src.begin()), std::make_move_iterator(src.end()));
    };
    auto joinLines = [](BlockLine& target, BlockLine&& from) {
        moveAppend(target.tokens, std::move(from.tokens));
        moveAppend(target.insignificants, std::move(from.insignificants));
    };

    auto parseBlock =
        [&](Column baseColumn, Column blockColumn, auto& parseBlock, auto& parseLine) -> BlockLiteralValue {
        //
        auto block = BlockLiteralValue{};

        while (true) {
            auto [type, line, position] = parseLine(baseColumn, blockColumn, parseBlock, parseLine);
            if (type == LineType::BlockStart) {
                while (true) {
                    auto [nextType, nextLine, nextPosition] = parseLine(baseColumn, blockColumn, parseBlock, parseLine);
                    if (nextType == LineType::BlockStart) {
                        joinLines(line, std::move(nextLine));
                        continue;
                    }
                    if (nextType == LineType::WithEnd) {
                        joinLines(line, std::move(nextLine));
                        block.lines.push_back(std::move(line));
                        if (!input) return block;
                        break;
                    }
                    if (nextType == LineType::Standalone) {
                        line.insignificants.push_back(MissingBlockEnd{{{}, position}});
                        block.lines.push_back(std::move(line));
                        block.lines.push_back(std::move(nextLine));
                        break;
                    }
                    if (nextType == LineType::LeaveBlock || nextType == LineType::BlockStartLeave) {
                        line.insignificants.push_back(MissingBlockEnd{{{}, position}});
                        block.lines.push_back(std::move(line));
                        block.lines.push_back(std::move(nextLine));
                        return block;
                    }
                }
            }
            else if (type == LineType::BlockStartLeave) {
                line.insignificants.push_back(MissingBlockEnd{{{}, position}});
                block.lines.push_back(std::move(line));
                return block;
            }
            else {
                block.lines.push_back(std::move(line));
                if (type == LineType::LeaveBlock) return block;
                if (type == LineType::WithEnd && !input) return block;
            }
        }
    };

    if (!input++) return {};
    auto blockColumn = Column{};
    if (input->startsOnNewLine()) {
        blockColumn = input->newLine().value.indentColumn;
    }
    auto block = parseBlock(Column{0}, blockColumn, parseBlock, parseLine);
    //    if (input) {
    //        // TODO(arBmind): report extra input
    //    }
    return {{}, block};
} // namespace nesting

} // namespace nesting
