#include "filterTokens.h"

#include "filter/Token.builder.h"
#include "filter/Token.ostream.h"

#include "scanner/Token.builder.h"
#include "scanner/Token.ostream.h"

#include "gtest/gtest.h"

#include <vector>

using namespace filter;

using ScannerTokens = std::vector<ScannerToken>;
using TokenLines = std::vector<TokenLine>;

struct TokensFilterData {
    const char* name{};
    ScannerTokens input{};
    TokenLines expected{};

    TokensFilterData(const char* name)
        : name{name} {}

    template<class... Tok>
    auto in(Tok&&... tok) && -> TokensFilterData {
        input = scanner::buildTokens(std::forward<Tok>(tok)...);
        return *this;
    }
    template<class... Lines>
    auto out(Lines&&... lines) && -> TokensFilterData {
        expected = filter::buildTokenLines(std::forward<Lines>(lines)...);
        return *this;
    }
};
static auto operator<<(std::ostream& out, const TokensFilterData& ttd) -> std::ostream& {
    out << "name: " << ttd.name << "\n";
    out << "input:\n";
    for (auto& t : ttd.input) out << t << '\n';
    out << "expected:\n";
    for (auto& t : ttd.expected) out << t << '\n';
    return out;
}

class TokenFilters : public testing::TestWithParam<TokensFilterData> {};

TEST_P(TokenFilters, FilterParser) {
    TokensFilterData data = GetParam();
    auto input = [&]() -> meta::CoEnumerator<ScannerToken> {
        for (const auto& t : data.input) {
            co_yield t;
        }
    }();
    auto tokLineGen = filterTokens(std::move(input));

    for (const auto& expectedLine : data.expected) {
        tokLineGen++;
        ASSERT_TRUE(static_cast<bool>(tokLineGen));
        const auto& tokenLine = *tokLineGen;
        ASSERT_EQ(tokenLine, expectedLine);
    }
}

INSTANTIATE_TEST_CASE_P(
    filterStart,
    TokenFilters,
    ::testing::Values(
        TokensFilterData("FilterOutStartingComment")
            .in(CommentLiteral{}, NewLineIndentation{}, id())
            .out(line().insignificants(CommentLiteral{}, NewLineIndentation{}).tokens(id())),

        TokensFilterData("FilterOutStartingIndentedComment")
            .in(NewLineIndentation{}, CommentLiteral{}, NewLineIndentation{}, View{})
            .out(line().insignificants(NewLineIndentation{}, CommentLiteral{}, NewLineIndentation{}).tokens(id())),

        TokensFilterData("FilterOutStartingCommentWhitespaceComment")
            .in(NewLineIndentation{},
                CommentLiteral{},
                WhiteSpaceSeparator{},
                CommentLiteral{},
                NewLineIndentation{},
                View{})
            .out(line()
                     .insignificants(
                         NewLineIndentation{},
                         CommentLiteral{},
                         WhiteSpaceSeparator{},
                         CommentLiteral{},
                         NewLineIndentation{})
                     .tokens(id())),

        TokensFilterData("FilterMultipleNewlines")
            .in(NewLineIndentation{}, NewLineIndentation{}, View{})
            .out(line().insignificants(NewLineIndentation{}, NewLineIndentation{}).tokens(id())),

        TokensFilterData("FilterEvenMoreNewlines")
            .in(NewLineIndentation{}, NewLineIndentation{}, NewLineIndentation{}, View{})
            .out(
                line().insignificants(NewLineIndentation{}, NewLineIndentation{}, NewLineIndentation{}).tokens(id())) //
        ),
    [](const ::testing::TestParamInfo<TokensFilterData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    filterEnd,
    TokenFilters,
    ::testing::Values(
        TokensFilterData("FilterOutFinalComment")
            .in(NewLineIndentation{}, View{}, CommentLiteral{})
            .out(line().insignificants(NewLineIndentation{}, CommentLiteral{}).tokens(id())),

        TokensFilterData("FilterOutFinalWhitespace")
            .in(NewLineIndentation{}, View{}, WhiteSpaceSeparator{})
            .out(line().insignificants(NewLineIndentation{}, WhiteSpaceSeparator{}).tokens(id())),

        TokensFilterData("FilterOutFinalNewline")
            .in(NewLineIndentation{}, View{}, NewLineIndentation{})
            .out(
                line().insignificants(NewLineIndentation{}).tokens(id()),
                line().insignificants(NewLineIndentation{})) //
        ),
    [](const ::testing::TestParamInfo<TokensFilterData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    semicolons,
    TokenFilters,
    ::testing::Values(
        TokensFilterData("FilterOutStartSemicolon")
            .in(NewLineIndentation{}, SemicolonSeparator{}, View{}, CommentLiteral{})
            .out(line().insignificants(NewLineIndentation{}, SemicolonSeparator{}, CommentLiteral{}).tokens(id())),

        TokensFilterData("SplitLineOnSemicolon")
            .in(View{}, SemicolonSeparator{}, View{}, CommentLiteral{})
            .out(
                line().tokens(id()), //
                line().insignificants(SemicolonSeparator{}, CommentLiteral{}).tokens(id())),

        TokensFilterData("FilterOutSemicolonLineEnd")
            .in(View{}, SemicolonSeparator{}, NewLineIndentation{}, View{})
            .out(
                line().tokens(id()), //
                line().insignificants(SemicolonSeparator{}, NewLineIndentation{}).tokens(id())) //
        ),
    [](const ::testing::TestParamInfo<TokensFilterData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    blocks,
    TokenFilters,
    ::testing::Values(
        [] {
            auto begin = View{"begin"};
            return TokensFilterData("MutateIdentifierBlockStart")
                .in(NewLineIndentation{}, View{begin}, ColonSeparator{}, NewLineIndentation{})
                .out(
                    line().insignificants(NewLineIndentation{}, BlockStartColon{}).tokens(id(begin)),
                    line().insignificants(NewLineIndentation{}));
        }(),
        [] {
            auto begin = View{"begin"};
            return TokensFilterData("MutateIdentifierBlockStartWithComment")
                .in(View{begin},
                    ColonSeparator{},
                    scanner::WhiteSpaceSeparator{},
                    scanner::CommentLiteral{},
                    NewLineIndentation{})
                .out(
                    line().tokens(id(begin)).insignificants(BlockStartColon{}, WhiteSpaceSeparator{}, CommentLiteral{}),
                    line().insignificants(NewLineIndentation{}));
        }(),
        [] {
            auto end = View{"end"};
            return TokensFilterData("MutateBlockEnd")
                .in(NewLineIndentation{},
                    View{},
                    ColonSeparator{},
                    NewLineIndentation{},
                    View{end},
                    NewLineIndentation{})
                .out(
                    line().insignificants(NewLineIndentation{}, BlockStartColon{}).tokens(id()),
                    line().insignificants(NewLineIndentation{}, blockEnd(end)),
                    line().insignificants(NewLineIndentation{}));
        }(),
        [] {
            auto end = View{"end"};
            return TokensFilterData("UnexpectedColon")
                .in(NewLineIndentation{}, ColonSeparator{}, NewLineIndentation{}, View{end}, NewLineIndentation{})
                .out(
                    line().insignificants(NewLineIndentation{}, UnexpectedColon{}),
                    line().insignificants(NewLineIndentation{}, blockEnd(end)),
                    line().insignificants(NewLineIndentation{}));
        }() //
        ),
    [](const ::testing::TestParamInfo<TokensFilterData>& inf) { return inf.param.name; });
