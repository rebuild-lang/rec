#include "scanner/NumberScanner.h"

#include "scanner/TokenOutput.h"

#include "gtest/gtest.h"

using namespace scanner;

struct NumberData {
    String input;
    // expected:
    String content;
    Column endColumn;
    Radix radix;
    String integerPart;
    String fractionalPart;
    Sign exponentSign;
    String exponentPart;
};
static auto operator<<(std::ostream& o, const NumberData& nd) -> std::ostream& {
    return o << nd.input << " => " << nd.content << " @" << nd.endColumn.v << '\n' //
             << "NumberLiteral: " << to_string(nd.radix) << ' ' << nd.integerPart;
}

class NumberScanners : public testing::TestWithParam<NumberData> {};

TEST_P(NumberScanners, all) {
    NumberData param = GetParam();
    auto f = File{String{"testfile"}, param.input};
    auto input = FileInput{f};

    const auto lit = NumberScanner::scan(input.peek().value(), input);

    const auto& value = lit.value;
    EXPECT_EQ(param.radix, value.radix);
    EXPECT_EQ(param.integerPart, to_string(value.integerPart));
    EXPECT_EQ(param.fractionalPart, to_string(value.fractionalPart));
    EXPECT_EQ(param.exponentSign, value.exponentSign);
    EXPECT_EQ(param.exponentPart, to_string(value.exponentPart));

    EXPECT_EQ(param.content, strings::to_string(lit.range.text));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.range.begin);
    const auto endPosition = Position{Line{1}, param.endColumn};
    EXPECT_EQ(endPosition, lit.range.end);
}

INSTANTIATE_TEST_CASE_P( //
    fields,
    NumberScanners,
    ::testing::Values( //
        NumberData{String{"12'3"},
                   String{"12'3"},
                   Column{5},
                   Radix::decimal,
                   String{"123"},
                   String{},
                   Sign::positive,
                   String{}},
        NumberData{String{"0.12'3"},
                   String{"0.12'3"},
                   Column{7},
                   Radix::decimal,
                   String{},
                   String{"123"},
                   Sign::positive,
                   String{}},
        NumberData{String{"0e12'3"},
                   String{"0e12'3"},
                   Column{7},
                   Radix::decimal,
                   String{},
                   String{},
                   Sign::positive,
                   String{"123"}},
        NumberData{String{"1.2e-3"},
                   String{"1.2e-3"},
                   Column{7},
                   Radix::decimal,
                   String{"1"},
                   String{"2"},
                   Sign::negative,
                   String{"3"}}));
INSTANTIATE_TEST_CASE_P( //
    zeros,
    NumberScanners,
    ::testing::Values( //
        NumberData{String{"0"}, String{"0"}, Column{2}, Radix::decimal, String{}, String{}, Sign::positive, String{}},
        NumberData{String{"0."}, String{"0."}, Column{3}, Radix::decimal, String{}, String{}, Sign::positive, String{}},
        NumberData{String{"0x0"}, String{"0x0"}, Column{4}, Radix::hex, String{}, String{}, Sign::positive, String{}},
        NumberData{String{"0o0"}, String{"0o0"}, Column{4}, Radix::octal, String{}, String{}, Sign::positive, String{}},
        NumberData{
            String{"0b0"}, String{"0b0"}, Column{4}, Radix::binary, String{}, String{}, Sign::positive, String{}},
        NumberData{String{"0x0."}, String{"0x0."}, Column{5}, Radix::hex, String{}, String{}, Sign::positive, String{}},
        NumberData{String{"0'.e0"},
                   String{"0'.e0"},
                   Column{6},
                   Radix::decimal,
                   String{},
                   String{},
                   Sign::positive,
                   String{}}));
INSTANTIATE_TEST_CASE_P( //
    hex,
    NumberScanners,
    ::testing::Values( //
        NumberData{
            String{"0xF'F"}, String{"0xF'F"}, Column{6}, Radix::hex, String{"FF"}, String{}, Sign::positive, String{}},
        NumberData{String{"0.12"},
                   String{"0.12"},
                   Column{5},
                   Radix::decimal,
                   String{},
                   String{"12"},
                   Sign::positive,
                   String{}},
        NumberData{String{"0b012"},
                   String{"0b01"},
                   Column{5},
                   Radix::binary,
                   String{"1"},
                   String{},
                   Sign::positive,
                   String{}}));

class NumberFailures : public testing::TestWithParam<String> {};

TEST_P(NumberFailures, all) {
    String param = GetParam();

    auto f = File{String{"testfile"}, param};
    auto input = FileInput{f};
    const auto lit = NumberScanner::scan(input.peek().value(), input);

    // ASSERT_TRUE(tok.holds<NumberLiteral>());
    const auto& value = lit.value;
    EXPECT_FALSE(value);
}

INSTANTIATE_TEST_CASE_P( //
    all,
    NumberFailures,
    ::testing::Values( //
        String{"0x"}, //
        String{"0o"}, //
        String{"0o9"}, //
        String{"0b"}, //
        String{"0b2"}, //
        String{"0.e"} //
        ));
