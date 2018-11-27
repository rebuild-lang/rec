#include "scanner/extractComment.h"

#include "scanner/Token.ostream.h"

#include "gtest/gtest.h"

using namespace text;

struct CommentData {
    std::string name;
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

    const auto lit = scanner::extractComment(input, param.tabStops);

    // EXPECT_EQ(param.content, strings::to_string(lit.range.view));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    // EXPECT_EQ(beginPosition, lit.range.begin);
    // EXPECT_EQ(param.end, lit.range.end);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    CommentScanners,
    ::testing::Values( //
        CommentData{"line", String{"# line \n"}, Column{8}, String{"# line "}, {Line{1}, Column{8}}},
        CommentData{"terminating", String{"#comment"}, Column{8}, String{"#comment"}, {Line{1}, Column{9}}},
        CommentData{"withTab", String{"#\tcomment #\n"}, Column{4}, String{"#\tcomment #"}, {Line{1}, Column{14}}},
        CommentData{"withTab2", String{"#  \tcomment #\n"}, Column{4}, String{"#  \tcomment #"}, {Line{1}, Column{14}}},
        CommentData{
            "block", String{"#end#\ncomment\n#end#"}, Column{4}, String{"#end#\ncomment\n#end#"}, {Line{3}, Column{6}}},
        CommentData{"blockTab",
                    String{"#end#\ncomment\n\t#end#"},
                    Column{4},
                    String{"#end#\ncomment\n\t#end#"},
                    {Line{3}, Column{10}}}),
    [](const ::testing::TestParamInfo<CommentData>& inf) { return inf.param.name; });
