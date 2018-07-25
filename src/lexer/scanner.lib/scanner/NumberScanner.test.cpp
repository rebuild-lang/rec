#include "scanner/NumberScanner.h"

#include "scanner/Token.ostream.h"

#include "gtest/gtest.h"

using namespace scanner;
using String = strings::String;
using namespace text;

struct NumberData {
    std::string name;
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

    const auto lit = extractNumber(input.peek().value(), input);

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
        NumberData{"intOnly",
                   String{"12'3"},
                   String{"12'3"},
                   Column{5},
                   Radix::decimal,
                   String{"123"},
                   String{},
                   Sign::positive,
                   String{}},
        NumberData{"fractionOnly",
                   String{"0.12'3"},
                   String{"0.12'3"},
                   Column{7},
                   Radix::decimal,
                   String{},
                   String{"123"},
                   Sign::positive,
                   String{}},
        NumberData{"exponentOnly",
                   String{"0e12'3"},
                   String{"0e12'3"},
                   Column{7},
                   Radix::decimal,
                   String{},
                   String{},
                   Sign::positive,
                   String{"123"}},
        NumberData{"negativeExponent",
                   String{"1.2e-3"},
                   String{"1.2e-3"},
                   Column{7},
                   Radix::decimal,
                   String{"1"},
                   String{"2"},
                   Sign::negative,
                   String{"3"}}),
    [](const ::testing::TestParamInfo<NumberData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P( //
    zeros,
    NumberScanners,
    ::testing::Values( //
        NumberData{"justZero",
                   String{"0"},
                   String{"0"},
                   Column{2},
                   Radix::decimal,
                   String{},
                   String{},
                   Sign::positive,
                   String{}},
        NumberData{
            "dot", String{"0."}, String{"0."}, Column{3}, Radix::decimal, String{}, String{}, Sign::positive, String{}},
        NumberData{
            "hex", String{"0x0"}, String{"0x0"}, Column{4}, Radix::hex, String{}, String{}, Sign::positive, String{}},
        NumberData{"octal",
                   String{"0o0"},
                   String{"0o0"},
                   Column{4},
                   Radix::octal,
                   String{},
                   String{},
                   Sign::positive,
                   String{}},
        NumberData{"binary",
                   String{"0b0"},
                   String{"0b0"},
                   Column{4},
                   Radix::binary,
                   String{},
                   String{},
                   Sign::positive,
                   String{}},
        NumberData{"hexFloat",
                   String{"0x0."},
                   String{"0x0."},
                   Column{5},
                   Radix::hex,
                   String{},
                   String{},
                   Sign::positive,
                   String{}},
        NumberData{"exponent",
                   String{"0'.e0"},
                   String{"0'.e0"},
                   Column{6},
                   Radix::decimal,
                   String{},
                   String{},
                   Sign::positive,
                   String{}}),
    [](const ::testing::TestParamInfo<NumberData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P( //
    hex,
    NumberScanners,
    ::testing::Values( //
        NumberData{"int",
                   String{"0xF'F"},
                   String{"0xF'F"},
                   Column{6},
                   Radix::hex,
                   String{"FF"},
                   String{},
                   Sign::positive,
                   String{}},
        NumberData{"float",
                   String{"0.12"},
                   String{"0.12"},
                   Column{5},
                   Radix::decimal,
                   String{},
                   String{"12"},
                   Sign::positive,
                   String{}},
        NumberData{"binary",
                   String{"0b01"},
                   String{"0b01"},
                   Column{5},
                   Radix::binary,
                   String{"1"},
                   String{},
                   Sign::positive,
                   String{}}),
    [](const ::testing::TestParamInfo<NumberData>& inf) { return inf.param.name; });

struct NumberFailureData {
    std::string name;
    String input;
};
static auto operator<<(std::ostream& o, const NumberFailureData& nd) -> std::ostream& { return o << nd.input; }

class NumberFailures : public testing::TestWithParam<NumberFailureData> {};

TEST_P(NumberFailures, all) {
    NumberFailureData param = GetParam();

    auto f = File{String{"testfile"}, param.input};
    auto input = FileInput{f};
    const auto lit = extractNumber(input.peek().value(), input);

    const auto& value = lit.value;
    EXPECT_FALSE(value);
}

INSTANTIATE_TEST_CASE_P( //
    all,
    NumberFailures,
    ::testing::Values( //
        NumberFailureData{"hexNothing", String{"0x"}}, //
        NumberFailureData{"octalNothing", String{"0o"}}, //
        NumberFailureData{"octalOutOfBounds", String{"0o9"}}, //
        NumberFailureData{"octalSecondOutOfBounds", String{"0o19"}}, //
        NumberFailureData{"binaryNothing", String{"0b"}}, //
        NumberFailureData{"binaryOutOfBounds", String{"0b2"}}, //
        NumberFailureData{"fractionNothing", String{"0.e"}},
        NumberFailureData{"positiveFractionNothing", String{"1e+"}},
        NumberFailureData{"negativeFractionNothing", String{"1.e-"}},
        NumberFailureData{"notTerminated", String{"0z"}}),
    [](const ::testing::TestParamInfo<NumberFailureData>& inf) { return inf.param.name; });
