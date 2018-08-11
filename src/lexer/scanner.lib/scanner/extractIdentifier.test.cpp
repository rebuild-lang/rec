#include "scanner/extractIdentifier.h"

#include "scanner/Token.ostream.h"

#include "gtest/gtest.h"

using namespace scanner;
using namespace text;

struct IdentifierData {
    std::string name;
    String input;
    // expected:
    String content;
    Column endColumn;
};
static auto operator<<(std::ostream& o, const IdentifierData& id) -> std::ostream& {
    return o << id.input << " => " << id.content << " @" << id.endColumn.v << ' ' //
             << "Identifier";
}

class IdentifierScanners : public testing::TestWithParam<IdentifierData> {};

TEST_P(IdentifierScanners, all) {
    IdentifierData param = GetParam();
    auto f = File{String{"testfile"}, param.input};
    auto input = FileInput{f};
    input.peek();

    const auto optTok = extractIdentifier(input);

    if (param.content.isEmpty()) {
        EXPECT_FALSE(optTok);
        return;
    }
    const auto tok = optTok.value();

    ASSERT_TRUE(tok.holds<IdentifierLiteral>());

    const auto& lit = tok.get<IdentifierLiteral>();
    EXPECT_EQ(param.content, strings::to_string(lit.range.view));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.range.begin);
    const auto endPosition = Position{Line{1}, param.endColumn};
    EXPECT_EQ(endPosition, lit.range.end);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    IdentifierScanners,
    ::testing::Values( //
        IdentifierData{"noId", String{"1"}, String{}, Column{3}},
        IdentifierData{"id", String{"id"}, String{"id"}, Column{3}},
        IdentifierData{"upperCase", String{"HiLo"}, String{"HiLo"}, Column{5}},
        IdentifierData{"long_id", String{"i3_v("}, String{"i3_v"}, Column{5}},
        IdentifierData{"dotStart", String{".id"}, String{".id"}, Column{4}},
        IdentifierData{"dotStops", String{"id.2"}, String{"id"}, Column{3}}),
    [](const ::testing::TestParamInfo<IdentifierData>& inf) { return inf.param.name; });
