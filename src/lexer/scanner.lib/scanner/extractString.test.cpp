#include "scanner/extractString.h"

#include "scanner/Token.ostream.h"

#include <gtest/gtest.h>

using namespace scanner;
using namespace text;

struct StringData {
    std::string name;
    String input;
    // expected:
    String content;
    Position end;
    // string
    String text;
};
static auto operator<<(std::ostream& o, const StringData& sd) -> std::ostream& {
    return o << sd.input << " => " << sd.content << " @" << sd.end << "\n"
             << "StringLiteral: " << sd.text;
}

class StringScanners : public testing::TestWithParam<StringData> {};

TEST_P(StringScanners, all) {
    StringData param = GetParam();
    auto f = File{String{"testfile"}, param.input};
    auto input = FileInput{f};
    input.peek();

    const auto lit = extractString(input);

    EXPECT_EQ(param.text, strings::to_string(lit.value.text));

    EXPECT_EQ(param.content, strings::to_string(lit.range.view));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.range.begin);
    EXPECT_EQ(param.end, lit.range.end);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    StringScanners,
    ::testing::Values( //
        StringData{"empty", String{R"("")"}, String{R"("")"}, {Line{1}, Column{3}}, String{}},
        StringData{"letters", String{R"("hello")"}, String{R"("hello")"}, {Line{1}, Column{8}}, String{R"(hello)"}},
        StringData{"whitespace", String{R"("he lo")"}, String{R"("he lo")"}, {Line{1}, Column{8}}, String{R"(he lo)"}},
        StringData{"newline", String{"\"h \nlo\""}, String{"\"h \nlo\""}, {Line{2}, Column{4}}, String{"hlo"}},
        StringData{"emptyNewline", String{"\"\n\""}, String{"\"\n\""}, {Line{2}, Column{2}}, String{""}},
        StringData{"escapedNewline", //
                   String{R"("he\nlo")"},
                   String{R"("he\nlo")"},
                   {Line{1}, Column{9}},
                   String{"he\nlo"}},
        StringData{"escapedQuote", String{R"("\"")"}, String{R"("\"")"}, {Line{1}, Column{5}}, String{"\""}},
        StringData{"rawEmpty", String{R"("""""")"}, String{R"("""""")"}, {Line{1}, Column{7}}, String{}},
        StringData{"rawLetters", //
                   String{R"("""hello""")"},
                   String{R"("""hello""")"},
                   {Line{1}, Column{12}},
                   String{"hello"}},
        StringData{"rawWithNewline",
                   String{"\"\"\"line1  \n  line2\"\"\""},
                   String{"\"\"\"line1  \n  line2\"\"\""},
                   {Line{2}, Column{11}},
                   String{"line1\n  line2"}},
        StringData{"rawWithQuotes",
                   String{R"("""hello "world"""")"},
                   String{R"("""hello "world"""")"},
                   {Line{1}, Column{20}},
                   String{"hello \"world\""}},
        StringData{"rawWithDoubleQuotes",
                   String{R"("""hello ""unit""""")"},
                   String{R"("""hello ""unit""""")"},
                   {Line{1}, Column{21}},
                   String{"hello \"\"unit\"\""}},
        StringData{"rawWithTripleQuotes",
                   String{R"("""hello """"""unit"""""")"},
                   String{R"("""hello """"""unit"""""")"},
                   {Line{1}, Column{26}},
                   String{"hello \"\"\"unit\"\"\""}},
        StringData{"missingTerminator", //
                   String{R"("tailing)"},
                   String{R"("tailing)"},
                   {Line{1}, Column{9}},
                   String{"tailing"}},
        StringData{"hexUnicode", //
                   String{R"("\x10abCD")"},
                   String{R"("\x10abCD")"},
                   {Line{1}, Column{11}},
                   String{[] {
                       auto vec = std::vector<String::Data>{};
                       strings::CodePoint{0x10abCD}.utf8_encode(vec);
                       return vec;
                   }()}},
        StringData{"decimalUnicode", //
                   String{R"("\u1114111")"},
                   String{R"("\u1114111")"},
                   {Line{1}, Column{12}},
                   String{[] {
                       auto vec = std::vector<String::Data>{};
                       strings::CodePoint{0x10FFFF}.utf8_encode(vec);
                       return vec;
                   }()}}),
    [](const ::testing::TestParamInfo<StringData>& inf) { return inf.param.name; });

struct StringCompareError : StringError {

    bool operator==(const StringError& o) const {
        return o.kind == kind && o.input.isContentEqual(input) && o.position == position;
    }
    bool operator!=(const StringError& o) const { return !(*this == o); }
};
static auto operator<<(std::ostream& out, const StringCompareError& error) -> std::ostream& {
    return out << static_cast<const StringError&>(error);
}
static auto operator<<(std::ostream& out, const std::vector<StringCompareError>& errors) -> std::ostream& {
    for (const auto& error : errors) {
        out << "  error: " << error << '\n';
    }
    return out;
}
static bool operator==(const std::vector<StringCompareError>& l, const StringErrors& r) {
    auto l_i = l.begin();
    auto r_i = r.begin();
    for (; l_i != l.end() && r_i != r.end(); l_i++, r_i++) {
        if (*l_i != *r_i) return false;
    }
    return (l_i != l.end()) == (r_i != r.end());
}

struct StringErrorData {
    std::string name;
    String input;
    // expected:
    String content;
    std::vector<StringCompareError> errors;
    Position end;
    // string
    String text;
};
static auto operator<<(std::ostream& o, const StringErrorData& sd) -> std::ostream& {
    return o << sd.input << " => " << sd.content << " @" << sd.end << "\n"
             << "StringLiteral: " << sd.text << "\n"
             << "Errors: " << sd.errors;
}

class StringErrorScanners : public testing::TestWithParam<StringErrorData> {};

TEST_P(StringErrorScanners, all) {
    StringErrorData param = GetParam();
    auto f = File{String{"testfile"}, param.input};
    auto input = FileInput{f};
    input.peek();

    const auto lit = extractString(input);

    EXPECT_EQ(param.text, strings::to_string(lit.value.text));
    EXPECT_EQ(param.errors, lit.value.errors);

    EXPECT_EQ(param.content, strings::to_string(lit.range.view));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.range.begin);
    EXPECT_EQ(param.end, lit.range.end);
}

INSTANTIATE_TEST_CASE_P( //
    error_cases,
    StringErrorScanners,
    ::testing::Values( //
        StringErrorData{"earlyTerminated",
                        String{"\"ab"},
                        String{"\"ab"},
                        {StringCompareError{StringError::Kind::EndOfInput, //
                                            View{""},
                                            Position{Line{1}, Column{4}}}},
                        {Line{1}, Column{4}},
                        String{"ab"}},
        StringErrorData{"invalidCodePoint",
                        String{"\"\0\""},
                        String{"\"\0\""},
                        {StringCompareError{StringError::Kind::InvalidControl, //
                                            View{"\0"},
                                            Position{Line{1}, Column{2}}}},
                        {Line{1}, Column{4}},
                        String{}},
        StringErrorData{"invalidEscape",
                        String{"\"\\?\""},
                        String{"\"\\?\""},
                        {StringCompareError{StringError::Kind::InvalidEscape, //
                                            View{"\\?"},
                                            Position{Line{1}, Column{2}}}},
                        {Line{1}, Column{5}},
                        String{}},
        StringErrorData{"noHexUnicode",
                        String{"\"\\x\""},
                        String{"\"\\x\""},
                        {StringCompareError{StringError::Kind::InvalidHexUnicode, //
                                            View{"\\x"},
                                            Position{Line{1}, Column{2}}}},
                        {Line{1}, Column{5}},
                        String{}},
        StringErrorData{"toobigHexUnicode",
                        String{"\"\\x110000\""},
                        String{"\"\\x110000\""},
                        {StringCompareError{StringError::Kind::InvalidHexUnicode, //
                                            View{"\\x110000"},
                                            Position{Line{1}, Column{2}}}},
                        {Line{1}, Column{11}},
                        String{}},
        StringErrorData{"noDecimalUnicode",
                        String{"\"\\uA\""},
                        String{"\"\\uA\""},
                        {StringCompareError{StringError::Kind::InvalidDecimalUnicode, //
                                            View{"\\u"},
                                            Position{Line{1}, Column{2}}}},
                        {Line{1}, Column{6}},
                        String{"A"}}),
    [](const ::testing::TestParamInfo<StringErrorData>& inf) { return inf.param.name; });
