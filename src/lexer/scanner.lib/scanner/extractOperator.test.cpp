#include "scanner/extractOperator.h"

#include "scanner/Token.ostream.h"

#include "gtest/gtest.h"

using namespace scanner;
using namespace text;

struct OperatorData {
    std::string name;
    String input;
    // expected:
    String content;
    Column endColumn;
};
static auto operator<<(std::ostream& o, const OperatorData& id) -> std::ostream& {
    return o << id.input << " => " << id.content << " @" << id.endColumn.v << ' ' //
             << "Identifier";
}

class OperatorScanners : public testing::TestWithParam<OperatorData> {};

TEST_P(OperatorScanners, all) {
    OperatorData param = GetParam();
    auto f = File{String{"testfile"}, param.input};
    auto input = FileInput{f};
    input.peek();

    const auto optTok = extractOperator(input);

    if (param.content.isEmpty()) {
        EXPECT_FALSE(optTok);
        return;
    }
    const auto tok = optTok.value();

    ASSERT_TRUE(tok.holds<OperatorLiteral>());

    const auto& lit = tok.get<OperatorLiteral>();
    EXPECT_EQ(param.content, strings::to_string(lit.range.text));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.range.begin);
    const auto endPosition = Position{Line{1}, param.endColumn};
    EXPECT_EQ(endPosition, lit.range.end);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    OperatorScanners,
    ::testing::Values( //
        OperatorData{"noOp", String{"id"}, String{}, Column{1}},
        OperatorData{"single", String{"+"}, String{"+"}, Column{2}},
        OperatorData{"combo", String{"*/+"}, String{"*/+"}, Column{4}},
        OperatorData{"enclosed", String{"{add}"}, String{"{add}"}, Column{6}},
        OperatorData{"nested", String{"{add{nest}more}"}, String{"{add{nest}more}"}, Column{16}},
        OperatorData{"notClosed", String{"+{a}{b"}, String{"+{a}"}, Column{5}},
        OperatorData{"extraChars", String{"½¼⅓²©®-"}, String{"½¼⅓²©®-"}, Column{8}}),
    [](const ::testing::TestParamInfo<OperatorData>& inf) { return inf.param.name; });
