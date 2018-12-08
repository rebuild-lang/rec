#include "scanner/extractOperator.h"

#include <scanner/Token.ostream.h>

#include <strings/utf8Decode.h>

#include <gtest/gtest.h>

using namespace scanner;
using namespace text;

struct OperatorData {
    std::string name;
    String input;
    // expected:
    String content;
};
static auto operator<<(std::ostream& o, const OperatorData& id) -> std::ostream& {
    return o << id.input << " => " << id.content << ' ' //
             << "Identifier";
}

class OperatorScanners : public testing::TestWithParam<OperatorData> {};

TEST_P(OperatorScanners, all) {
    OperatorData param = GetParam();
    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        auto column = Column{};
        for (auto decoded : strings::utf8Decode(param.input)) {
            co_yield decoded.visit(
                [&](strings::DecodedCodePoint& dcp) -> DecodedPosition {
                    auto cpp = CodePointPosition{dcp.input, dcp.cp, Position{Line{1}, column}};
                    ++column;
                    return cpp;
                },
                [&](strings::DecodedError& de) -> DecodedPosition {
                    auto dep = DecodedErrorPosition{de.input, Position{Line{1}, column}};
                    return dep;
                });
        }
    }();
    decoder++;
    auto cpp = (*decoder).get<CodePointPosition>();
    decoder++;
    const OptToken optTok = extractOperator(cpp, decoder);

    if (param.content.isEmpty()) {
        EXPECT_FALSE(optTok);
        return;
    }
    ASSERT_TRUE(optTok);

    auto tok = optTok.value();
    ASSERT_TRUE(tok.holds<OperatorLiteral>());

    const auto& lit = tok.get<OperatorLiteral>();
    EXPECT_EQ(param.content, strings::to_string(lit.input));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.position);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    OperatorScanners,
    ::testing::Values( //
        OperatorData{"noOp", String{"id"}, String{}},
        OperatorData{"single", String{"+"}, String{"+"}},
        OperatorData{"combo", String{"*/+"}, String{"*/+"}},
        OperatorData{"enclosed", String{"{add}"}, String{"{add}"}},
        OperatorData{"nested", String{"{add{nest}more}"}, String{"{add{nest}more}"}},
        OperatorData{"notClosed", String{"+{a}{b "}, String{"+{a}{b"}},
        OperatorData{"extraChars", String{"½¼⅓²©®-"}, String{"½¼⅓²©®-"}}),
    [](const ::testing::TestParamInfo<OperatorData>& inf) { return inf.param.name; });
