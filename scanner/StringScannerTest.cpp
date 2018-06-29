#include "scanner/StringScanner.h"

#include "scanner/TokenOutput.h"

#include <gtest/gtest.h>

using namespace scanner;

struct StringData {
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

    const auto tok = StringScanner::scan(input);

    ASSERT_TRUE(tok.holds<StringLiteral>());

    const auto& lit = tok.get<StringLiteral>();
    EXPECT_EQ(param.text, strings::to_string(lit.value.text));

    EXPECT_EQ(param.content, strings::to_string(lit.range.text));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.range.begin);
    EXPECT_EQ(param.end, lit.range.end);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    StringScanners,
    ::testing::Values( //
        StringData{String{R"("")"}, // empty
                   String{R"("")"},
                   {Line{1}, Column{3}},
                   String{}},
        StringData{String{R"("hello")"}, // no whitespace
                   String{R"("hello")"},
                   {Line{1}, Column{8}},
                   String{R"(hello)"}},
        StringData{String{R"("he lo")"}, // with whitespace
                   String{R"("he lo")"},
                   {Line{1}, Column{8}},
                   String{R"(he lo)"}},
        StringData{String{"\"h \nlo\""}, // newline
                   String{"\"h \nlo\""},
                   {Line{2}, Column{4}},
                   String{"hlo"}},
        StringData{String{"\"\n\""}, // empty newline
                   String{"\"\n\""},
                   {Line{2}, Column{2}},
                   String{""}},
        StringData{String{R"("he\nlo")"}, // escaped newline
                   String{R"("he\nlo")"},
                   {Line{1}, Column{9}},
                   String{"he\nlo"}},
        StringData{String{R"("\"")"}, // escaped quote
                   String{R"("\"")"},
                   {Line{1}, Column{5}},
                   String{"\""}},
        StringData{String{R"("""""")"}, // empty raw string
                   String{R"("""""")"},
                   {Line{1}, Column{7}},
                   String{}},
        StringData{String{R"("""hello""")"}, // raw string
                   String{R"("""hello""")"},
                   {Line{1}, Column{12}},
                   String{"hello"}},
        StringData{String{"\"\"\"line1  \n  line2\"\"\""}, // raw with tailing whitespace newline
                   String{"\"\"\"line1  \n  line2\"\"\""},
                   {Line{2}, Column{11}},
                   String{"line1\n  line2"}},
        StringData{String{R"("""hello "world"""")"}, // raw with quotes
                   String{R"("""hello "world"""")"},
                   {Line{1}, Column{20}},
                   String{"hello \"world\""}},
        StringData{String{R"("""hello ""unit""""")"}, // raw with 2x quotes
                   String{R"("""hello ""unit""""")"},
                   {Line{1}, Column{21}},
                   String{"hello \"\"unit\"\""}},
        StringData{String{R"("""hello """"""unit"""""")"}, // raw with 3x quotes
                   String{R"("""hello """"""unit"""""")"},
                   {Line{1}, Column{26}},
                   String{"hello \"\"\"unit\"\"\""}},
        StringData{String{R"("tailing)"}, // missing terminator
                   String{R"("tailing)"},
                   {Line{1}, Column{9}},
                   String{"tailing"}}));
