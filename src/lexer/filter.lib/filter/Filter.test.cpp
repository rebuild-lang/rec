#include "Filter.h"

#include "filter/Token.builder.h"
#include "filter/Token.ostream.h"

#include "scanner/Token.builder.h"

#include "gtest/gtest.h"

#include <vector>

using namespace filter;

using ScannerTokens = std::vector<ScannerToken>;
using FilterTokens = std::vector<Token>;

struct TokensTransformData {
    const char* name{};
    ScannerTokens input{};
    FilterTokens expected{};

    TokensTransformData(const char* name)
        : name{name} {}

    template<class... Tok>
    auto in(Tok&&... tok) && -> TokensTransformData {
        input = scanner::buildTokens(std::forward<Tok>(tok)...);
        return *this;
    }
    template<class... Tok>
    auto out(Tok&&... tok) && -> TokensTransformData {
        expected = filter::buildTokens(std::forward<Tok>(tok)...);
        return *this;
    }
};
static auto operator<<(std::ostream& out, const TokensTransformData& ttd) -> std::ostream& {
    out << "name: " << ttd.name << "\n";
    out << "input:\n";
    for (auto& t : ttd.input) out << t << '\n';
    out << "expected:\n";
    for (auto& t : ttd.expected) out << t << '\n';
    return out;
}

class TokenTransformations : public testing::TestWithParam<TokensTransformData> {};

TEST_P(TokenTransformations, FilterParser) {
    TokensTransformData data = GetParam();
    auto input = [&]() -> meta::CoEnumerator<ScannerToken> {
        for (const auto& t : data.input) {
            co_yield t;
        }
    }();

    auto tokGen = Filter::parse(std::move(input));

    for (const auto& et : data.expected) {
        tokGen++;
        ASSERT_TRUE(static_cast<bool>(tokGen));
        const auto& tok = *tokGen;
        ASSERT_EQ(tok, et);
    }
}

INSTANTIATE_TEST_CASE_P(
    filterStart,
    TokenTransformations,
    ::testing::Values(
        TokensTransformData("Filter out starting comment")
            .in(scanner::CommentLiteral{}, NewLineIndentation{}, View{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensTransformData("Filter out starting indented comment")
            .in(NewLineIndentation{}, scanner::CommentLiteral{}, NewLineIndentation{}, View{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensTransformData("Filter out starting comment whitespace comment")
            .in(NewLineIndentation{},
                scanner::CommentLiteral{},
                scanner::WhiteSpaceSeparator{},
                scanner::CommentLiteral{},
                NewLineIndentation{},
                View{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensTransformData("Filter multiple newlines")
            .in(NewLineIndentation{}, NewLineIndentation{}, View{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensTransformData("Filter even more newlines")
            .in(NewLineIndentation{}, NewLineIndentation{}, NewLineIndentation{}, View{})
            .out(NewLineIndentation{}, id().bothSeparated()) //
        ));

INSTANTIATE_TEST_CASE_P(
    filterEnd,
    TokenTransformations,
    ::testing::Values(
        TokensTransformData("Filter out final comment")
            .in(NewLineIndentation{}, View{}, scanner::CommentLiteral{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensTransformData("Filter out final whitespace")
            .in(NewLineIndentation{}, View{}, scanner::WhiteSpaceSeparator{})
            .out(NewLineIndentation{}, id().bothSeparated()),
        TokensTransformData("Filter out final newline")
            .in(NewLineIndentation{}, View{}, NewLineIndentation{})
            .out(NewLineIndentation{}, id().bothSeparated()) //
        ));

INSTANTIATE_TEST_CASE_P(
    blocks,
    TokenTransformations,
    ::testing::Values(
        TokensTransformData("Mutate identifier block start")
            .in(NewLineIndentation{}, View{"begin"}, ColonSeparator{}, NewLineIndentation{})
            .out(NewLineIndentation{}, id("begin").bothSeparated(), BlockStartIndentation{}),

        TokensTransformData("Mutate identifier block start with comment")
            .in(View{"begin"},
                ColonSeparator{},
                scanner::WhiteSpaceSeparator{},
                scanner::CommentLiteral{},
                NewLineIndentation{})
            .out(NewLineIndentation{}, id("begin").bothSeparated(), BlockStartIndentation{}),

        TokensTransformData("Mutate block end")
            .in(NewLineIndentation{}, ColonSeparator{}, NewLineIndentation{}, View{"end"}, NewLineIndentation{})
            .out(NewLineIndentation{}, BlockStartIndentation{}, BlockEndIndentation{}) //
        ));

INSTANTIATE_TEST_CASE_P(
    neighbors,
    TokenTransformations,
    ::testing::Values(
        TokensTransformData("With white spaces")
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

        TokensTransformData("border cases")
            .in(View{"left"}, View{"right"})
            .out(NewLineIndentation{}, id("left").leftSeparated(), id("right").rightSeparated()),

        TokensTransformData("Brackets")
            .in(BracketOpen{}, View{"left"}, View{"right"}, BracketClose{}, View{"stuck"}, BracketOpen{})
            .out(
                NewLineIndentation{},
                BracketOpen{},
                id("left").leftSeparated(),
                id("right").rightSeparated(),
                BracketClose{},
                id("stuck"),
                BracketOpen{}),

        TokensTransformData("Comma")
            .in(scanner::WhiteSpaceSeparator{}, View{"left"}, CommaSeparator{}, View{"right"})
            .out(NewLineIndentation{}, id("left").bothSeparated(), CommaSeparator{}, id("right").bothSeparated()),

        TokensTransformData("Semicolon")
            .in(scanner::WhiteSpaceSeparator{}, View{"left"}, SemicolonSeparator{}, View{"right"})
            .out(
                NewLineIndentation{},
                id("left").bothSeparated(),
                SemicolonSeparator{},
                id("right").bothSeparated()) //
        ));
