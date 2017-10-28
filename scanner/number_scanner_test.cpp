#include "scanner/number_scanner.h"

#include "gtest/gtest.h"

using namespace scanner;

struct number_data {
    string_t input;
    // expected:
    string_t content;
    column_t end_column;
    radix_t radix;
    string_t integer_part;
    string_t fractional_part;
    sign_t exponent_sign;
    string_t exponent_part;
};
std::ostream &operator<<(std::ostream &o, const number_data &nd) {
    return o << nd.input << " => " << nd.content << " @" << nd.end_column.v << '\n' //
             << "number_literal: " << to_string(nd.radix) << ' ' << nd.integer_part;
}

class number_scanners : public testing::TestWithParam<number_data> {};

TEST_P(number_scanners, all) {
    number_data param = GetParam();
    auto f = file_t{string_t{"testfile"}, param.input};
    auto input = file_input_t{f};

    auto tok = number_scanner::scan(input.peek_char().value(), input);

    EXPECT_EQ(param.content, strings::to_string(tok.range.text));
    constexpr const auto begin_position = position_t{line_t{1}, column_t{1}};
    EXPECT_EQ(begin_position, tok.range.begin_position);
    const auto end_position = position_t{line_t{1}, param.end_column};
    EXPECT_EQ(end_position, tok.range.end_position);

    ASSERT_TRUE(std::holds_alternative<number_literal_t>(tok.data));
    const number_literal_t &lit = std::get<number_literal_t>(tok.data);
    EXPECT_EQ(param.radix, lit.radix);
    EXPECT_EQ(param.integer_part, to_string(lit.integer_part));
    EXPECT_EQ(param.fractional_part, to_string(lit.fractional_part));
    EXPECT_EQ(param.exponent_sign, lit.exponent_sign);
    EXPECT_EQ(param.exponent_part, to_string(lit.exponent_part));
}

INSTANTIATE_TEST_CASE_P(fields, number_scanners,
                        ::testing::Values( //
                            number_data{string_t{"12'3"}, string_t{"12'3"}, column_t{5}, radix_t::decimal,
                                        string_t{"123"}, string_t{}, sign_t::positive, string_t{}},
                            number_data{string_t{"0.12'3"}, string_t{"0.12'3"}, column_t{7}, radix_t::decimal,
                                        string_t{}, string_t{"123"}, sign_t::positive, string_t{}},
                            number_data{string_t{"0e12'3"}, string_t{"0e12'3"}, column_t{7}, radix_t::decimal,
                                        string_t{}, string_t{}, sign_t::positive, string_t{"123"}},
                            number_data{string_t{"1.2e-3"}, string_t{"1.2e-3"}, column_t{7}, radix_t::decimal,
                                        string_t{"1"}, string_t{"2"}, sign_t::negative, string_t{"3"}}));
INSTANTIATE_TEST_CASE_P(zeros, number_scanners,
                        ::testing::Values( //
                            number_data{string_t{"0"}, string_t{"0"}, column_t{2}, radix_t::decimal, string_t{},
                                        string_t{}, sign_t::positive, string_t{}},
                            number_data{string_t{"0."}, string_t{"0."}, column_t{3}, radix_t::decimal, string_t{},
                                        string_t{}, sign_t::positive, string_t{}},
                            number_data{string_t{"0x0"}, string_t{"0x0"}, column_t{4}, radix_t::hex, string_t{},
                                        string_t{}, sign_t::positive, string_t{}},
                            number_data{string_t{"0o0"}, string_t{"0o0"}, column_t{4}, radix_t::octal, string_t{},
                                        string_t{}, sign_t::positive, string_t{}},
                            number_data{string_t{"0b0"}, string_t{"0b0"}, column_t{4}, radix_t::binary, string_t{},
                                        string_t{}, sign_t::positive, string_t{}},
                            number_data{string_t{"0x0."}, string_t{"0x0."}, column_t{5}, radix_t::hex, string_t{},
                                        string_t{}, sign_t::positive, string_t{}},
                            number_data{string_t{"0'.e0"}, string_t{"0'.e0"}, column_t{6}, radix_t::decimal, string_t{},
                                        string_t{}, sign_t::positive, string_t{}}));
INSTANTIATE_TEST_CASE_P(hex, number_scanners,
                        ::testing::Values( //
                            number_data{string_t{"0xF'F"}, string_t{"0xF'F"}, column_t{6}, radix_t::hex, string_t{"FF"},
                                        string_t{}, sign_t::positive, string_t{}},
                            number_data{string_t{"0.12"}, string_t{"0.12"}, column_t{5}, radix_t::decimal, string_t{},
                                        string_t{"12"}, sign_t::positive, string_t{}},
                            number_data{string_t{"0b012"}, string_t{"0b01"}, column_t{5}, radix_t::binary,
                                        string_t{"1"}, string_t{}, sign_t::positive, string_t{}}));

class number_failures : public testing::TestWithParam<string_t> {};

TEST_P(number_failures, all) {
    string_t param = GetParam();

    auto f = file_t{string_t{"testfile"}, param};
    auto input = file_input_t{f};
    auto tok = number_scanner::scan(input.peek_char().value(), input);

    ASSERT_TRUE(std::holds_alternative<number_literal_t>(tok.data));
    const number_literal_t &lit = std::get<number_literal_t>(tok.data);
    EXPECT_FALSE(lit);
}

INSTANTIATE_TEST_CASE_P(all, number_failures,
                        ::testing::Values(  //
                            string_t{"0x"}, //
                            string_t{"0o"}, //
                            string_t{"0b"}, //
                            string_t{"0.e"} //
                            ));
