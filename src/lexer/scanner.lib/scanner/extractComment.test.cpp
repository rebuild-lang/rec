#include "scanner/extractComment.h"

#include "scanner/Token.ostream.h"

#include "gtest/gtest.h"

using namespace scanner;
using namespace text;

struct CommentData {
    std::string name;
    String input;
    // expected:
    String content;
};
static auto operator<<(std::ostream& o, const CommentData& cd) -> std::ostream& {
    return o << cd.input << " => " << cd.content << "\n"
             << "CommentLiteral";
}

class CommentScanners : public testing::TestWithParam<CommentData> {};

TEST_P(CommentScanners, all) {
    CommentData param = GetParam();
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
                auto cpp = CodePointPosition{view, cp, position};
                co_yield cpp;
            }
            ++column;
        }
    }();
    decoder++;
    auto cpp = (*decoder).get<CodePointPosition>();
    decoder++;
    const CommentLiteral lit = extractComment(cpp, decoder);

    const auto& value = lit.value;
    EXPECT_TRUE(value.decodeErrors.empty());
    EXPECT_EQ(param.content, strings::to_string(lit.input));
    constexpr const auto beginPosition = Position{Line{1}, Column{1}};
    EXPECT_EQ(beginPosition, lit.position);
}

INSTANTIATE_TEST_CASE_P( //
    examples,
    CommentScanners,
    ::testing::Values( //
        CommentData{"line", String{"# line \n"}, String{"# line "}},
        CommentData{"terminating", String{"#comment"}, String{"#comment"}},
        CommentData{"withTab", String{"#\tcomment #\n"}, String{"#\tcomment #"}},
        CommentData{"withTab2", String{"#  \tcomment #\n"}, String{"#  \tcomment #"}},
        CommentData{"block", String{"#end#\ncomment\n#end#"}, String{"#end#\ncomment\n#end#"}},
        CommentData{"blockTab", String{"#end#\ncomment\n\t#end#"}, String{"#end#\ncomment\n\t#end#"}}),
    [](const ::testing::TestParamInfo<CommentData>& inf) { return inf.param.name; });
