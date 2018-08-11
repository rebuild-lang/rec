#include "filterTokens.h"

#include "filter/Token.builder.h"
#include "filter/Token.ostream.h"

#include "scanner/Token.builder.h"

#include "gtest/gtest.h"

#include <vector>

using namespace filter;

using ScannerTokens = std::vector<ScannerToken>;
using FilterTokens = std::vector<Token>;

struct TokensFilterData {
    const char* name{};
    ScannerTokens input{};
    FilterTokens expected{};

    TokensFilterData(const char* name)
        : name{name} {}

    template<class... Tok>
    auto in(Tok&&... tok) && -> TokensFilterData {
        input = scanner::buildTokens(std::forward<Tok>(tok)...);
        return *this;
    }
    template<class... Tok>
    auto out(Tok&&... tok) && -> TokensFilterData {
        expected = filter::buildTokens(std::forward<Tok>(tok)...);
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

    auto tokGen = filterTokens(std::move(input));

    for (const auto& et : data.expected) {
        tokGen++;
        ASSERT_TRUE(static_cast<bool>(tokGen));
        const auto& tok = *tokGen;
        ASSERT_EQ(tok, et);
    }
}

INSTANTIATE_TEST_CASE_P(
    filterStart,
    TokenFilters,
    ::testing::Values(
        TokensFilterData("FilterOutStartingComment")
            .in(scanner::CommentLiteral{}, NewLineIndentation{}, View{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensFilterData("FilterOutStartingIndentedComment")
            .in(NewLineIndentation{}, scanner::CommentLiteral{}, NewLineIndentation{}, View{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensFilterData("FilterOutStartingCommentWhitespaceComment")
            .in(NewLineIndentation{},
                scanner::CommentLiteral{},
                scanner::WhiteSpaceSeparator{},
                scanner::CommentLiteral{},
                NewLineIndentation{},
                View{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensFilterData("FilterMultipleNewlines")
            .in(NewLineIndentation{}, NewLineIndentation{}, View{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensFilterData("FilterEvenMoreNewlines")
            .in(NewLineIndentation{}, NewLineIndentation{}, NewLineIndentation{}, View{})
            .out(NewLineIndentation{}, id().bothSeparated()) //
        ),
    [](const ::testing::TestParamInfo<TokensFilterData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    filterEnd,
    TokenFilters,
    ::testing::Values(
        TokensFilterData("FilterOutFinalComment")
            .in(NewLineIndentation{}, View{}, scanner::CommentLiteral{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensFilterData("FilterOutFinalWhitespace")
            .in(NewLineIndentation{}, View{}, scanner::WhiteSpaceSeparator{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensFilterData("FilterOutFinalNewline")
            .in(NewLineIndentation{}, View{}, NewLineIndentation{})
            .out(NewLineIndentation{}, id().bothSeparated()) //
        ),
    [](const ::testing::TestParamInfo<TokensFilterData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    blocks,
    TokenFilters,
    ::testing::Values(
        TokensFilterData("MutateIdentifierBlockStart")
            .in(NewLineIndentation{}, View{"begin"}, ColonSeparator{}, NewLineIndentation{})
            .out(NewLineIndentation{}, id("begin").bothSeparated(), BlockStartIndentation{}),

        TokensFilterData("MutateIdentifierBlockStartWithComment")
            .in(View{"begin"},
                ColonSeparator{},
                scanner::WhiteSpaceSeparator{},
                scanner::CommentLiteral{},
                NewLineIndentation{})
            .out(NewLineIndentation{}, id("begin").bothSeparated(), BlockStartIndentation{}),

        TokensFilterData("MutateBlockEnd")
            .in(NewLineIndentation{}, ColonSeparator{}, NewLineIndentation{}, View{"end"}, NewLineIndentation{})
            .out(NewLineIndentation{}, BlockStartIndentation{}, BlockEndIndentation{}) //
        ),
    [](const ::testing::TestParamInfo<TokensFilterData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    neighbors,
    TokenFilters,
    ::testing::Values(
        TokensFilterData("WithWhiteSpaces")
            .in(scanner::WhiteSpaceSeparator{},
                View{"left"},
                View{"middle"},
                View{"right"},
                scanner::WhiteSpaceSeparator{},
                View{"free"},
                scanner::WhiteSpaceSeparator{})
            .out(
                NewLineIndentation{},
                id("left").leftSeparated(),
                id("middle"),
                id("right").rightSeparated(),
                id("free").bothSeparated()),

        TokensFilterData("BorderCases")
            .in(View{"left"}, View{"right"})
            .out(NewLineIndentation{}, id("left").leftSeparated(), id("right").rightSeparated()),

        TokensFilterData("Brackets")
            .in(BracketOpen{}, View{"left"}, View{"right"}, BracketClose{}, View{"stuck"}, BracketOpen{})
            .out(
                NewLineIndentation{},
                BracketOpen{},
                id("left").leftSeparated(),
                id("right").rightSeparated(),
                BracketClose{},
                id("stuck"),
                BracketOpen{}),

        TokensFilterData("Comma")
            .in(scanner::WhiteSpaceSeparator{}, View{"left"}, CommaSeparator{}, View{"right"})
            .out(NewLineIndentation{}, id("left").bothSeparated(), CommaSeparator{}, id("right").bothSeparated()),

        TokensFilterData("Semicolon")
            .in(scanner::WhiteSpaceSeparator{}, View{"left"}, SemicolonSeparator{}, View{"right"})
            .out(
                NewLineIndentation{},
                id("left").bothSeparated(),
                SemicolonSeparator{},
                id("right").bothSeparated()) //
        ),
    [](const ::testing::TestParamInfo<TokensFilterData>& inf) { return inf.param.name; });
