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
};
static auto operator<<(std::ostream& o, const IdentifierData& id) -> std::ostream& {
    return o << id.input << " => " << id.content << ' ' //
             << "Identifier";
}

class IdentifierScanners : public testing::TestWithParam<IdentifierData> {};

TEST_P(IdentifierScanners, all) {
    IdentifierData param = GetParam();
    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        auto column = Column{};
        for (auto& chr : param.input) {
            auto cp = CodePoint{static_cast<uint32_t>(chr)};
            auto cpp = CodePointPosition{View{&chr, &chr + 1}, cp, Position{Line{1}, column}};
            co_yield cpp;
            ++column;
        }
    }();
    decoder++;
    auto cpp = (*decoder).get<CodePointPosition>();
    decoder++;
    const OptToken optTok = extractIdentifier(cpp, decoder);

    if (param.content.isEmpty()) {
        EXPECT_FALSE(optTok);
        return;
    }
    const auto tok = optTok.value();

    ASSERT_TRUE(tok.holds<IdentifierLiteral>());

    const auto& lit = tok.get<IdentifierLiteral>();
    EXPECT_EQ(param.content, strings::to_string(lit.input));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.position);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    IdentifierScanners,
    ::testing::Values( //
        IdentifierData{"noId", String{"1"}, String{}},
        IdentifierData{"id", String{"id"}, String{"id"}},
        IdentifierData{"upperCase", String{"HiLo"}, String{"HiLo"}},
        IdentifierData{"long_id", String{"i3_v("}, String{"i3_v"}},
        IdentifierData{"dotStart", String{".id"}, String{".id"}},
        IdentifierData{"dotStops", String{"id.2"}, String{"id"}}),
    [](const ::testing::TestParamInfo<IdentifierData>& inf) { return inf.param.name; });
