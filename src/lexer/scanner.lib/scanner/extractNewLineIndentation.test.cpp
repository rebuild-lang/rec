#include "scanner/extractNewLineIndentation.h"

#include <scanner/Token.ostream.h>

#include <gtest/gtest.h>

using namespace scanner;
using namespace text;

struct NewLineData {
    std::string name;
    View input;
    NewLineIndentationValue expected;
};
static auto operator<<(std::ostream& o, const NewLineData& nld) -> std::ostream& {
    return o << nld.input << " => " << nld.expected;
}

class NewLineScanners : public testing::TestWithParam<NewLineData> {};

TEST_P(NewLineScanners, all) {
    NewLineData param = GetParam();
    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        auto column = Column{};
        for (auto& chr : param.input) {
            auto view = View{&chr, &chr + 1};
            auto position = Position{Line{1}, column};
            if (chr == '\n') {
                auto nlp = NewlinePosition{view, position};
                co_yield nlp;
                column = Column{};
                continue;
            }
            if (chr == 'E') {
                auto dep = DecodedErrorPosition{view, position};
                co_yield dep;
                continue;
            }
            if (chr == '\t')
                column = Column{(column.v - (column.v - 1) % 8) + 8};
            else
                ++column;
            auto cp = CodePoint{static_cast<uint32_t>(chr)};
            auto cpp = CodePointPosition{view, position, cp, Position{Line{1}, column}};
            co_yield cpp;
        }
    }();
    decoder++;
    auto nlp = NewlinePosition{};
    auto state = ExtractNewLineState{};
    const NewLineIndentation tok = extractNewLineIndentation(nlp, decoder, state);

    const auto& value = tok.value;
    EXPECT_EQ(param.expected, value);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    NewLineScanners,
    ::testing::Values( //
        NewLineData{"empty", View{""}, {{}, Column{1}}},
        NewLineData{"spaces", View{"  "}, {{}, Column{3}}},
        NewLineData{"stops", View{"  x"}, {{}, Column{3}}},
        NewLineData{"tabs", View{"\t\t"}, {{}, Column{17}}},
        [] {
            auto input = View(" \t\t");
            return NewLineData{
                "mixed", //
                input,
                {{MixedIndentCharacter{input.skipBytes<1>().firstBytes<1>(), Position{{}, Column{2}}}}, Column{17}} //
            };
        }(),
        [] {
            auto input = View(" E x");
            return NewLineData{
                "consumesErrors",
                input,
                {{DecodedErrorPosition{input.skipBytes<1>().firstBytes<1>(), Position{{}, Column{2}}}}, Column{3}} //
            };
        }()),
    [](const ::testing::TestParamInfo<NewLineData>& inf) { return inf.param.name; });
