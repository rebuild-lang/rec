#include "scanner/OperatorScanner.h"

#include "scanner/TokenOutput.h"

#include "gtest/gtest.h"

using namespace scanner;

struct OperatorData {
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

    const auto optTok = OperatorScanner::scan(input);

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
        OperatorData{String{"id"}, // no op
                     String{},
                     Column{1}},
        OperatorData{String{"+"}, // single
                     String{"+"},
                     Column{2}},
        OperatorData{String{"*/+"}, // combo
                     String{"*/+"},
                     Column{4}},
        OperatorData{String{"{add}"}, // enclosed
                     String{"{add}"},
                     Column{6}},
        OperatorData{String{"{add{nest}more}"}, // nested
                     String{"{add{nest}more}"},
                     Column{16}},
        OperatorData{String{"+{a}{b"}, // not closed
                     String{"+{a}"},
                     Column{5}},
        OperatorData{String{"½¼⅓²©®-"}, // extra chars
                     String{"½¼⅓²©®-"},
                     Column{8}}));
