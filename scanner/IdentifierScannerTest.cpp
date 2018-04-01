#include "scanner/IdentifierScanner.h"

#include "scanner/TokenOutput.h"

#include "gtest/gtest.h"

using namespace scanner;

struct IdentifierData {
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

    const auto optTok = IdentifierScanner::scan(input);

    if (param.content.isEmpty()) {
        EXPECT_FALSE(optTok);
        return;
    }
    const auto tok = optTok.value();

    EXPECT_EQ(param.content, strings::to_string(tok.range.text));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, tok.range.begin);
    const auto endPosition = Position{Line{1}, param.endColumn};
    EXPECT_EQ(endPosition, tok.range.end);

    ASSERT_TRUE(tok.data.holds<IdentifierLiteral>());
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    IdentifierScanners,
    ::testing::Values( //
        IdentifierData{String{"1"}, // no id
                       String{},
                       Column{3}},
        IdentifierData{String{"id"}, // id
                       String{"id"},
                       Column{3}},
        IdentifierData{String{"HiLo"}, // upper case
                       String{"HiLo"},
                       Column{5}},
        IdentifierData{String{"i3_v("}, // id
                       String{"i3_v"},
                       Column{5}},
        IdentifierData{String{".id"}, // dot start
                       String{".id"},
                       Column{4}},
        IdentifierData{String{"id.2"}, // dot stops
                       String{"id"},
                       Column{3}}));
