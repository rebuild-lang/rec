#include "nesting/nestTokens.h"

#include "nesting/Token.builder.h"
#include "nesting/Token.ostream.h"

#include "filter/Token.builder.h"
#include "filter/Token.ostream.h"

#include "gtest/gtest.h"

using namespace nesting;

using FilterTokens = std::vector<FilterToken>;

struct NestTokensData {
    const char* name{};
    FilterTokens input{};
    BlockLiteral expected{};

    NestTokensData(const char* name)
        : name{name} {}

    template<class... Tok>
    auto in(Tok&&... tok) && -> NestTokensData {
        input = filter::buildTokens(std::forward<Tok>(tok)...);
        return *this;
    }
    template<class... Lines>
    auto out(Lines&&... lines) && -> NestTokensData {
        expected = BlockLiteral{{}, {}, {{std::forward<Lines>(lines)...}}};
        return *this;
    }
};
static auto operator<<(std::ostream& out, const NestTokensData& ttd) -> std::ostream& {
    out << "name: " << ttd.name << "\n";
    out << "input:\n";
    for (auto& t : ttd.input) out << t << '\n';
    out << "expected:\n";
    out << ttd.expected << '\n';
    return out;
}

class NestTransformation : public testing::TestWithParam<NestTokensData> {};

TEST_P(NestTransformation, BlockParser) {
    NestTokensData data = GetParam();
    auto input = [&]() -> meta::CoEnumerator<FilterToken> {
        for (const auto& t : data.input) {
            co_yield t;
        }
    }();

    auto blk = nesting::nestTokens(std::move(input));

    ASSERT_EQ(blk, data.expected);
}

INSTANTIATE_TEST_CASE_P(
    examples,
    NestTransformation,
    ::testing::Values( //
        NestTokensData("OneLine")
            .in(filter::id("if"),
                filter::id("a"),
                newLine(Column{4}),
                filter::op("&&"),
                filter::id("b"),
                blockStart(Column{4}),
                filter::id("print_a"),
                newLine(Column{4}),
                filter::id("print_b"),
                newLine(Column{1}),
                filter::id("else"),
                blockStart(Column{4}),
                filter::id("print_c"),
                blockEnd(Column{1}))
            .out(buildTokens(
                id("if"),
                id("a"),
                op("&&"),
                id("b"),
                blk(Column{4}, buildTokens(id("print_a")), buildTokens(id("print_b"))),
                id("else"),
                blk(Column{4}, buildTokens(id("print_c"))))), //
        NestTokensData("TwoBlockLines")
            .in(filter::id("module"),
                filter::id("A"),
                blockStart(Column{4}),
                blockEnd(Column{1}),
                filter::id("module"),
                filter::id("B"),
                blockStart(Column{4}),
                filter::id("say_hello"),
                blockEnd(Column{1}))
            .out(
                buildTokens(id("module"), id("A"), blk(Column{4})),
                buildTokens(id("module"), id("B"), blk(Column{4}, buildTokens(id("say_hello"))))) //
        ),
    [](const ::testing::TestParamInfo<NestTokensData>& inf) { return inf.param.name; });
