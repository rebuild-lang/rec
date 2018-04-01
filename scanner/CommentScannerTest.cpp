#include "scanner/CommentScanner.h"

#include "scanner/TokenOutput.h"

#include "gtest/gtest.h"

using namespace scanner;

struct CommentData {
    String input;
    Column tabStops;
    // expected:
    String content;
    Position end;
};
static auto operator<<(std::ostream& o, const CommentData& cd) -> std::ostream& {
    return o << cd.input << " [tabStop=" << cd.tabStops.v << "] => " << cd.content << " @" << cd.end << "\n"
             << "CommentLiteral";
}

class CommentScanners : public testing::TestWithParam<CommentData> {};

TEST_P(CommentScanners, all) {
    CommentData param = GetParam();
    auto f = File{String{"testfile"}, param.input};
    auto input = FileInput{f};
    input.peek();

    const auto tok = CommentScanner::scan(input, param.tabStops);

    EXPECT_EQ(param.content, strings::to_string(tok.range.text));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, tok.range.begin);
    EXPECT_EQ(param.end, tok.range.end);

    ASSERT_TRUE(tok.data.holds<CommentLiteral>());
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    CommentScanners,
    ::testing::Values( //
        CommentData{String{"# line \n"}, // line comment
                    Column{8},
                    String{"# line "},
                    {Line{1}, Column{8}}},
        CommentData{String{"#comment"}, // terminating comment
                    Column{8},
                    String{"#comment"},
                    {Line{1}, Column{9}}},
        CommentData{String{"#\tcomment #\n"}, // comment with tab
                    Column{4},
                    String{"#\tcomment #"},
                    {Line{1}, Column{14}}},
        CommentData{String{"#  \tcomment #\n"}, // comment with tab
                    Column{4},
                    String{"#  \tcomment #"},
                    {Line{1}, Column{14}}},
        CommentData{String{"#end#\ncomment\n#end#"}, // block comment
                    Column{4},
                    String{"#end#\ncomment\n#end#"},
                    {Line{3}, Column{6}}}));
