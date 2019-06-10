#include "scanner/extractString.h"

#include <scanner/Token.ostream.h>

#include <gtest/gtest.h>

using namespace scanner;
using namespace text;

struct StringData {
    std::string name;
    String input;
    // expected:
    String content;
    // string
    String text;
};
static auto operator<<(std::ostream& o, const StringData& sd) -> std::ostream& {
    return o << sd.input << " => " << sd.content << "\n"
             << "StringLiteral: " << sd.text;
}

class StringScanners : public testing::TestWithParam<StringData> {};

TEST_P(StringScanners, all) {
    StringData param = GetParam();
    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        auto column = Column{};
        for (auto& chr : param.input) {
            auto view = View{&chr, &chr + 1};
            auto position = Position{Line{1}, column};
            if (chr == '\n') {
                auto nlp = NewlinePosition{view, position};
                co_yield nlp;
            }
            else {
                auto cp = CodePoint{static_cast<uint32_t>(chr)};
                auto cpp = CodePointPosition{view, position, cp};
                co_yield cpp;
            }
            ++column;
        }
    }();
    decoder++;
    auto cpp = (*decoder).get<CodePointPosition>();
    decoder++;
    const StringLiteral lit = extractString(cpp, decoder);

    const auto& value = lit.value;
    EXPECT_TRUE(value.errors.empty());
    EXPECT_EQ(param.text, strings::to_string(value.text));

    EXPECT_EQ(param.content, strings::to_string(lit.input));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.position);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    StringScanners,
    ::testing::Values( //
        StringData{"empty", String{R"("")"}, String{R"("")"}, String{}},
        StringData{"letters", String{R"("hello")"}, String{R"("hello")"}, String{R"(hello)"}},
        StringData{"whitespace", String{R"("he lo")"}, String{R"("he lo")"}, String{R"(he lo)"}},
        StringData{"newline", String{"\"h \nlo\""}, String{"\"h \nlo\""}, String{"hlo"}},
        StringData{"emptyNewline", String{"\"\n\""}, String{"\"\n\""}, String{""}},
        StringData{"escapedNewline", //
                   String{R"("he\nlo")"},
                   String{R"("he\nlo")"},
                   String{"he\nlo"}},
        StringData{"escapedQuote", String{R"("\"")"}, String{R"("\"")"}, String{"\""}},
        StringData{"rawEmpty", String{R"("""""")"}, String{R"("""""")"}, String{}},
        StringData{"rawLetters", //
                   String{R"("""hello""")"},
                   String{R"("""hello""")"},
                   String{"hello"}},
        StringData{"rawWithNewline",
                   String{"\"\"\"line1  \n  line2\"\"\""},
                   String{"\"\"\"line1  \n  line2\"\"\""},
                   String{"line1\n  line2"}},
        StringData{"rawWithQuotes",
                   String{R"("""hello "world"""")"},
                   String{R"("""hello "world"""")"},
                   String{"hello \"world\""}},
        StringData{"rawWithDoubleQuotes",
                   String{R"("""hello ""unit""""")"},
                   String{R"("""hello ""unit""""")"},
                   String{"hello \"\"unit\"\""}},
        StringData{"rawWithTripleQuotes",
                   String{R"("""hello """"""unit""""""""")"},
                   String{R"("""hello """"""unit""""""""")"},

                   String{"hello \"\"\"unit\"\"\""}},
        StringData{"hexUnicode", //
                   String{R"("\x10abCD")"},
                   String{R"("\x10abCD")"},
                   String{[] {
                       auto vec = std::vector<String::Char>{};
                       strings::CodePoint{0x10abCD}.utf8_encode(vec);
                       return vec;
                   }()}},
        StringData{"decimalUnicode", //
                   String{R"("\u1114111")"},
                   String{R"("\u1114111")"},
                   String{[] {
                       auto vec = std::vector<String::Char>{};
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
    // string
    String text;
};
static auto operator<<(std::ostream& o, const StringErrorData& sd) -> std::ostream& {
    return o << sd.input << " => " << sd.content << "\n"
             << "StringLiteral: " << sd.text << "\n"
             << "Errors: " << sd.errors;
}

class StringErrorScanners : public testing::TestWithParam<StringErrorData> {};

TEST_P(StringErrorScanners, all) {
    StringErrorData param = GetParam();
    auto decoder = [&]() -> meta::CoEnumerator<DecodedPosition> {
        auto column = Column{};
        for (auto& chr : param.input) {
            auto view = View{&chr, &chr + 1};
            auto position = Position{Line{1}, column};
            auto cp = CodePoint{static_cast<uint32_t>(chr)};
            auto cpp = CodePointPosition{view, position, cp};
            co_yield cpp;
            ++column;
        }
    }();
    decoder++;
    auto cpp = (*decoder).get<CodePointPosition>();
    decoder++;
    const StringLiteral lit = extractString(cpp, decoder);

    const auto& value = lit.value;
    EXPECT_EQ(param.errors, value.errors);
    EXPECT_EQ(param.text, strings::to_string(value.text));

    EXPECT_EQ(param.content, strings::to_string(lit.input));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.position);
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
                                            Position{Line{1}, Column{3}}}},
                        String{"ab"}},
        StringErrorData{"invalidCodePoint",
                        String{"\"\0\""},
                        String{"\"\0\""},
                        {StringCompareError{StringError::Kind::InvalidControl, //
                                            View{"\0"},
                                            Position{Line{1}, Column{2}}}},
                        String{}},
        StringErrorData{"invalidEscape",
                        String{"\"\\?\""},
                        String{"\"\\?\""},
                        {StringCompareError{StringError::Kind::InvalidEscape, //
                                            View{"\\?"},
                                            Position{Line{1}, Column{2}}}},
                        String{}},
        StringErrorData{"noHexUnicode",
                        String{"\"\\x\""},
                        String{"\"\\x\""},
                        {StringCompareError{StringError::Kind::InvalidHexUnicode, //
                                            View{"\\x"},
                                            Position{Line{1}, Column{2}}}},
                        String{}},
        StringErrorData{"toobigHexUnicode",
                        String{"\"\\x110000\""},
                        String{"\"\\x110000\""},
                        {StringCompareError{StringError::Kind::InvalidHexUnicode, //
                                            View{"\\x110000"},
                                            Position{Line{1}, Column{2}}}},
                        String{}},
        StringErrorData{"noDecimalUnicode",
                        String{"\"\\uA\""},
                        String{"\"\\uA\""},
                        {StringCompareError{StringError::Kind::InvalidDecimalUnicode, //
                                            View{"\\u"},
                                            Position{Line{1}, Column{2}}}},
                        String{"A"}}),
    [](const ::testing::TestParamInfo<StringErrorData>& inf) { return inf.param.name; });
