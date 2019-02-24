#include "scanner/extractNumber.h"

#include <scanner/Token.ostream.h>
#include <text/DecodedPosition.ostream.h>

#include <gtest/gtest.h>

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
    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        auto column = Column{};
        for (auto& chr : param.input) {
            auto cp = CodePoint{static_cast<uint32_t>(chr)};
            auto cpp = CodePointPosition{View{&chr, &chr + 1}, Position{Line{1}, column}, cp};
            co_yield cpp;
            ++column;
        }
    }();
    decoder++;
    auto cpp = (*decoder).get<CodePointPosition>();
    decoder++;
    const NumberLiteral lit = extractNumber(cpp, decoder);

    const auto& value = lit.value;
    EXPECT_TRUE(value.errors.empty());
    EXPECT_EQ(param.radix, value.radix);
    EXPECT_EQ(param.integerPart, to_string(value.integerPart));
    EXPECT_EQ(param.fractionalPart, to_string(value.fractionalPart));
    EXPECT_EQ(param.exponentSign, value.exponentSign);
    EXPECT_EQ(param.exponentPart, to_string(value.exponentPart));

    EXPECT_EQ(param.content, strings::to_string(lit.input));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.position);
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
    View input;
    NumberLiteralErrors errors;
};
static auto operator<<(std::ostream& o, const NumberFailureData& nd) -> std::ostream& { return o << nd.input; }

class NumberFailures : public testing::TestWithParam<NumberFailureData> {};

TEST_P(NumberFailures, all) {
    NumberFailureData param = GetParam();

    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        auto column = Column{};
        for (auto& chr : param.input) {
            auto cp = CodePoint{static_cast<uint32_t>(chr)};
            auto cpp = CodePointPosition{View{&chr, &chr + 1}, Position{Line{1}, column}, cp};
            co_yield cpp;
            ++column;
        }
    }();
    decoder++;
    auto cpp = (*decoder).get<CodePointPosition>();
    decoder++;
    const NumberLiteral lit = extractNumber(cpp, decoder);
    const auto& value = lit.value;
    EXPECT_TRUE(value.hasErrors());

    EXPECT_EQ(param.errors, value.errors);
}

INSTANTIATE_TEST_CASE_P( //
    all,
    NumberFailures,
    ::testing::Values( //
        NumberFailureData{"hexNothing", View{"0x"}, {NumberMissingValue{}}}, //
        NumberFailureData{"octalNothing", View{"0o"}, {NumberMissingValue{}}}, //
        [] {
            auto input = View{"0o9"};
            auto missingValue = NumberMissingValue{input.skipBytes<2>(), {Line{1}, Column{3}}};
            auto missingBoundary = NumberMissingBoundary{input.skipBytes<2>(), {Line{1}, Column{3}}};
            return NumberFailureData{"octalOutOfBounds", input, {missingValue, missingBoundary}};
        }(),
        [] {
            auto input = View{"0o19"};
            auto missingBoundary = NumberMissingBoundary{input.skipBytes<3>(), {Line{1}, Column{4}}};
            return NumberFailureData{"octalSecondOutOfBounds", input, {missingBoundary}};
        }(),
        NumberFailureData{"binaryNothing", View{"0b"}, {NumberMissingValue{}}}, //
        [] {
            auto input = View{"0b2"};
            auto missingValue = NumberMissingValue{input.skipBytes<2>(), {Line{1}, Column{3}}};
            auto missingBoundary = NumberMissingBoundary{input.skipBytes<2>(), {Line{1}, Column{3}}};
            return NumberFailureData{"binaryOutOfBounds", input, {missingValue, missingBoundary}};
        }(),
        NumberFailureData{"fractionNothing", View{"0.e"}, {NumberMissingExponent{}}},
        NumberFailureData{"positiveFractionNothing", View{"1e+"}, {NumberMissingExponent{}}},
        NumberFailureData{"negativeFractionNothing", View{"1.e-"}, {NumberMissingExponent{}}},
        [] {
            auto input = View{"0z"};
            auto missingBoundary = NumberMissingBoundary{input.skipBytes<1>(), {Line{1}, Column{2}}};
            return NumberFailureData{"notTerminated", input, {missingBoundary}};
        }()),
    [](const ::testing::TestParamInfo<NumberFailureData>& inf) { return inf.param.name; });

using DecodedPositions = std::vector<DecodedPosition>;

struct NumberDecodeErrorData {
    std::string name;
    DecodedPositions decoded;
    NumberLiteralErrors errors;
    Radix radix;
    String integerPart;
};
static auto operator<<(std::ostream& o, const NumberDecodeErrorData& nd) -> std::ostream& {
    return o << strings::joinEach(nd.decoded, " ");
}

class NumberDecodeErrors : public testing::TestWithParam<NumberDecodeErrorData> {};

TEST_P(NumberDecodeErrors, all) {
    NumberDecodeErrorData param = GetParam();

    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        for (auto decoded : param.decoded) {
            co_yield decoded;
        }
    }();
    decoder++;
    auto cpp = (*decoder).get<CodePointPosition>();
    decoder++;
    const NumberLiteral lit = extractNumber(cpp, decoder);

    const auto& value = lit.value;
    EXPECT_EQ(param.errors, value.errors);

    EXPECT_EQ(param.radix, value.radix);
    EXPECT_EQ(param.integerPart, to_string(value.integerPart));
}

INSTANTIATE_TEST_CASE_P( //
    all,
    NumberDecodeErrors,
    ::testing::Values( //
        [] {
            auto decodeError = DecodedErrorPosition{View{"xx"}, Position{}};
            return NumberDecodeErrorData{
                "ignorable",
                DecodedPositions{
                    CodePointPosition{View{"1"}, Position{}, CodePoint{'1'}},
                    decodeError,
                    CodePointPosition{View{"2"}, Position{Line{1}, Column{2}}, CodePoint{'2'}},
                },
                NumberLiteralErrors{
                    decodeError,
                },
                Radix::decimal,
                String("12") //
            };
        }(),
        [] {
            auto decodeError = DecodedErrorPosition{View{"xx"}, Position{}};
            return NumberDecodeErrorData{
                "beforeRadix",
                DecodedPositions{
                    CodePointPosition{View{"0"}, Position{}, CodePoint{'0'}},
                    decodeError,
                    CodePointPosition{View{"x"}, Position{Line{1}, Column{2}}, CodePoint{'x'}},
                    CodePointPosition{View{"F"}, Position{Line{1}, Column{3}}, CodePoint{'F'}},
                },
                NumberLiteralErrors{
                    decodeError,
                },
                Radix::hex,
                String("F") //
            };
        }(),
        [] {
            auto decodeError = DecodedErrorPosition{View{"xx"}, Position{}};
            return NumberDecodeErrorData{
                "beforeEnd",
                DecodedPositions{
                    CodePointPosition{View{"1"}, Position{}, CodePoint{'1'}},
                    CodePointPosition{View{"3"}, Position{}, CodePoint{'3'}},
                    decodeError,
                },
                NumberLiteralErrors{
                    decodeError,
                },
                Radix::decimal,
                String("13") //
            };
        }()),
    [](const ::testing::TestParamInfo<NumberDecodeErrorData>& inf) { return inf.param.name; });
