#include "nesting/nestTokens.h"

#include "nesting/Token.builder.h"
#include "nesting/Token.ostream.h"

#include "filter/Token.builder.h"
#include "filter/Token.ostream.h"

#include "gtest/gtest.h"

using namespace nesting;

using FilterTokenLine = filter::TokenLine;
using FilterTokenLines = std::vector<FilterTokenLine>;

struct NestTokensData {
    const char* name{};
    FilterTokenLines input{};
    BlockLiteral expected{};

    NestTokensData(const char* name)
        : name{name} {}

    template<class... Lines>
    auto in(Lines&&... lines) && -> NestTokensData {
        input = filter::buildTokenLines(std::forward<Lines>(lines)...);
        return std::move(*this);
    }
    template<class... Lines>
    auto out(Lines&&... lines) && -> NestTokensData {
        expected = BlockLiteral{{}, {buildBlockLines(std::forward<Lines>(lines)...)}};
        return std::move(*this);
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
    auto input = [&]() -> meta::CoEnumerator<FilterTokenLine> {
        for (const auto& t : data.input) co_yield t;
    }();

    auto blk = nesting::nestTokens(std::move(input));

    ASSERT_EQ(blk, data.expected);
}

INSTANTIATE_TEST_CASE_P(
    examples,
    NestTransformation,
    ::testing::Values(
        NestTokensData("Empty") //
            .in()
            .out(),
        [] {
            return NestTokensData("SimpleLine")
                .in(filter::line().tokens(filter::id(View{"print_a"}))) //
                .out(line().tokens(id(View{"print_a"})));
        }(),
        [] {
            return NestTokensData("TwoLines")
                .in(filter::line().tokens(filter::id(View{"print_a"})), //
                    filter::line().tokens(filter::id(View{"print_b"})))
                .out(
                    line().tokens(id(View{"print_a"})), //
                    line().tokens(id(View{"print_b"})));
        }(),
        [] {
            return NestTokensData("ContinuedLine")
                .in(filter::line().tokens(filter::id(View{"print_a"})),
                    filter::line().tokens(filter::id(View{"print_b"})).insignificants(filter::newLine(4)))
                .out(line().tokens(id(View{"print_a"}), id(View{"print_b"})).insignificants(filter::newLine(4)));
        }(),
        [] {
            return NestTokensData("EmptyBlock")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line().insignificants(filter::newLine(), filter::blockEnd(View{"end"})))
                .out(line()
                         .tokens(id(View{"begin"}))
                         .insignificants(BlockStartColon{})
                         .tokens(blk())
                         .insignificants(filter::newLine(), filter::blockEnd(View{"end"})));
        }(),
        [] {
            return NestTokensData("SimpleBlock")
                .in(filter::line().tokens(filter::id(View{"print_a"})).insignificants(filter::BlockStartColon{}),
                    //
                    filter::line().tokens(filter::id(View{"print_b"})).insignificants(filter::newLine(4)),
                    filter::line().insignificants(filter::newLine(), filter::blockEnd(View{"end"})))
                .out(line()
                         .tokens(id(View{"print_a"}))
                         .insignificants(BlockStartColon{})
                         .tokens(blk(line().tokens(id(View{"print_b"})).insignificants(filter::newLine(4))))
                         .insignificants(filter::newLine(), filter::blockEnd(View{"end"})));
        }(),
        [] {
            return NestTokensData("ContinuedSimpleBlock")
                .in(filter::line().tokens(filter::id(View{"code"})),
                    filter::line()
                        .tokens(filter::id(View{"continue"}))
                        .insignificants(filter::newLine(3), filter::BlockStartColon{}),
                    filter::line().tokens(filter::id(View{"line"})).insignificants(filter::newLine(4)),
                    filter::line().insignificants(filter::newLine(), filter::blockEnd(View{"end"})))
                .out(line()
                         .tokens(id(View{"code"}), id(View{"continue"}))
                         .insignificants(filter::newLine(3), BlockStartColon{})
                         .tokens(blk(line().tokens(id(View{"line"})).insignificants(filter::newLine(4))))
                         .insignificants(filter::newLine(), filter::blockEnd(View{"end"})));
        }()
        // NestTokensData("OneLine")
        //    .in(filter::line().insignificants().tokens(filter::id("if"), filter::id("a")),
        //        filter::line()
        //            .insignificants(filter::newLine(4), filter::ColonSeparator{})
        //            .tokens(filter::op("&&"), filter::id("b")),
        //        filter::line().insignificants(filter::newLine(4)).tokens(filter::id("print_a")),
        //        filter::line().insignificants(filter::newLine(4)).tokens(filter::id("print_b")),
        //        filter::line().insignificants(filter::newLine(),
        // filter::ColonSeparator{}).tokens(filter::id("else")),
        //        filter::line().insignificants(filter::newLine(4)).tokens(filter::id("print_c")),
        //        filter::line().insignificants(filter::newLine(), filter::id("end")))
        //    .out(line().tokens(
        //        id("if"),
        //        id("a"),
        //        op("&&"),
        //        id("b"),
        //        blk(line().tokens(id("print_a")), line().tokens(id("print_b"))),
        //        id("else"),
        //        blk(line().tokens(id("print_c"))))) //
        // NestTokensData("TwoBlockLines")
        //    .in(filter::id("module"),
        //        filter::id("A"),
        //        blockStart(Column{4}),
        //        blockEnd(Column{1}),
        //        filter::id("module"),
        //        filter::id("B"),
        //        blockStart(Column{4}),
        //        filter::id("say_hello"),
        //        blockEnd(Column{1}))
        //    .out(
        //        buildTokens(id("module"), id("A"), blk(Column{4})),
        //        buildTokens(id("module"), id("B"), blk(Column{4}, buildTokens(id("say_hello"))))) //
        ),
    [](const ::testing::TestParamInfo<NestTokensData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    missingEndErrors,
    NestTransformation,
    ::testing::Values(
        [] {
            return NestTokensData("WrongBlockStart")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line().insignificants(filter::newLine()).tokens(filter::id(View{"code"})))
                .out(
                    line().tokens(id(View{"begin"}), blk()).insignificants(BlockStartColon{}, MissingBlockEnd{}),
                    line().insignificants(filter::newLine()).tokens(id(View{"code"})));
        }(),
        [] {
            return NestTokensData("MissingBlockEnd")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line().insignificants(filter::newLine(5)).tokens(filter::id(View{"blockcode"})),
                    filter::line().insignificants(filter::newLine()).tokens(filter::id(View{"code"})))
                .out(
                    line()
                        .tokens(id(View{"begin"}))
                        .insignificants(BlockStartColon{})
                        .tokens(blk(line().insignificants(filter::newLine(5)).tokens(id(View{"blockcode"}))))
                        .insignificants(MissingBlockEnd{}),
                    line().insignificants(filter::newLine()).tokens(id(View{"code"})));
        }(),
        [] {
            return NestTokensData("MissingNestedEnd")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line()
                        .insignificants(filter::newLine(5))
                        .tokens(filter::id(View{"begin"}))
                        .insignificants(filter::BlockStartColon{}),
                    filter::line().insignificants(filter::newLine(9)).tokens(filter::id(View{"blockcode"})),
                    filter::line().insignificants(filter::newLine()).tokens(filter::id(View{"code"})))
                .out(
                    line()
                        .tokens(id(View{"begin"}))
                        .insignificants(BlockStartColon{})
                        .tokens(blk(
                            line()
                                .insignificants(filter::newLine(5))
                                .tokens(id(View{"begin"}))
                                .insignificants(BlockStartColon{})
                                .tokens(blk(line().insignificants(filter::newLine(9)).tokens(id(View{"blockcode"}))))
                                .insignificants(MissingBlockEnd{})))
                        .insignificants(MissingBlockEnd{}),
                    line().insignificants(filter::newLine()).tokens(id(View{"code"})));
        }()),
    [](const ::testing::TestParamInfo<NestTokensData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    extraEnd,
    NestTransformation,
    ::testing::Values(
        [] {
            return NestTokensData("EndInLineContinuation")
                .in(filter::line().tokens(filter::id(View{"code"})),
                    filter::line().insignificants(filter::newLine(5), filter::blockEnd(View{"end"})))
                .out(line()
                         .tokens(id(View{"code"}))
                         .insignificants(filter::newLine(5), UnexpectedBlockEnd{View{"end"}}));
        }(),
        [] {
            return NestTokensData("EndOnBlockIndent")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line().insignificants(filter::newLine(5)).tokens(filter::id(View{"blockcode"})),
                    filter::line().insignificants(filter::newLine(5), filter::blockEnd(View{"end"})),
                    filter::line().insignificants(filter::newLine()).tokens(filter::id(View{"code"})))
                .out(
                    line()
                        .tokens(id(View{"begin"}))
                        .insignificants(filter::BlockStartColon{})
                        .tokens(
                            blk(line().insignificants(filter::newLine(5)).tokens(id(View{"blockcode"})),
                                line().insignificants(filter::newLine(5), UnexpectedBlockEnd{View{"end"}})))
                        .insignificants(MissingBlockEnd{}),
                    line().insignificants(filter::newLine()).tokens(id(View{"code"})));
        }()),
    [](const ::testing::TestParamInfo<NestTokensData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    wrongBlockIndentation,
    NestTransformation,
    ::testing::Values(
        [] {
            return NestTokensData("CodeBetweenBlock")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line().insignificants(filter::newLine(5)).tokens(filter::id(View{"blockcode"})),
                    filter::line().insignificants(filter::newLine(3)).tokens(filter::id(View{"code"})),
                    filter::line().insignificants(filter::newLine(), filter::blockEnd(View{"end"})))
                .out(line()
                         .tokens(id(View{"begin"}))
                         .insignificants(filter::BlockStartColon{})
                         .tokens(blk(
                             line().insignificants(filter::newLine(5)).tokens(id(View{"blockcode"})),
                             line().insignificants(UnexpectedIndent{}, filter::newLine(3)).tokens(id(View{"code"}))))
                         .insignificants(filter::newLine(), filter::blockEnd(View{"end"})));
        }(),
        [] {
            return NestTokensData("OutdentedEnd")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line().insignificants(filter::newLine(5)).tokens(filter::id(View{"blockcode"})),
                    filter::line().insignificants(filter::newLine(3), filter::blockEnd(View{"end"})),
                    filter::line().insignificants(filter::newLine()).tokens(filter::id(View{"code"})))
                .out(
                    line()
                        .tokens(id(View{"begin"}))
                        .insignificants(filter::BlockStartColon{})
                        .tokens(
                            blk(line().insignificants(filter::newLine(5)).tokens(id(View{"blockcode"})),
                                line().insignificants(
                                    UnexpectedIndent{}, filter::newLine(3), UnexpectedBlockEnd{View{"end"}, {}})))
                        .insignificants(MissingBlockEnd{}),
                    line().insignificants(filter::newLine()).tokens(id(View{"code"})));
        }()),
    [](const ::testing::TestParamInfo<NestTokensData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    wrongLineIndentation,
    NestTransformation,
    ::testing::Values(
        [] {
            return NestTokensData("OutdentedLine")
                .in(filter::line().tokens(filter::id(View{"code"})),
                    filter::line().insignificants(filter::newLine(5)).tokens(filter::id(View{"linecode"})),
                    filter::line().insignificants(filter::newLine(3)).tokens(filter::id(View{"morecode"})))
                .out(line()
                         .tokens(id(View{"code"}))
                         .insignificants(filter::newLine(5))
                         .tokens(id(View{"linecode"}))
                         .tokens(blk(line()
                                         .insignificants(UnexpectedIndent{}, filter::newLine(3))
                                         .tokens(id(View{"morecode"})))));
        }(),
        [] {
            return NestTokensData("OutdentedEnd")
                .in(filter::line().tokens(filter::id(View{"code"})),
                    filter::line().insignificants(filter::newLine(5)).tokens(filter::id(View{"linecode"})),
                    filter::line().insignificants(filter::newLine(3), filter::blockEnd(View{"end"})))
                .out(line()
                         .tokens(id(View{"code"}))
                         .insignificants(filter::newLine(5))
                         .tokens(id(View{"linecode"}))
                         .tokens(blk(line().insignificants(
                             UnexpectedIndent{}, filter::newLine(3), UnexpectedBlockEnd{View{"end"}, {}}))));
        }()),
    [](const ::testing::TestParamInfo<NestTokensData>& inf) { return inf.param.name; });

INSTANTIATE_TEST_CASE_P(
    extraTokens,
    NestTransformation,
    ::testing::Values(
        [] {
            return NestTokensData("TokensAfterEmptyBlock")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line()
                        .insignificants(filter::newLine(), filter::blockEnd(View{"end"}))
                        .tokens(filter::id(View{"code"})))
                .out(line()
                         .tokens(id(View{"begin"}))
                         .insignificants(filter::BlockStartColon{})
                         .tokens(blk())
                         .insignificants(filter::newLine(), filter::blockEnd(View{"end"}), UnexpectedTokensAfterEnd{})
                         .tokens(id(View{"code"})));
        }(),
        [] {
            return NestTokensData("TokensAfterBlock")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line().insignificants(filter::newLine(5)).tokens(filter::id(View{"linecode"})),
                    filter::line()
                        .insignificants(filter::newLine(), filter::blockEnd(View{"end"}))
                        .tokens(filter::id(View{"code"})))
                .out(line()
                         .tokens(id(View{"begin"}))
                         .insignificants(filter::BlockStartColon{})
                         .tokens(blk(line().insignificants(filter::newLine(5)).tokens(filter::id(View{"linecode"}))))
                         .insignificants(filter::newLine(), filter::blockEnd(View{"end"}), UnexpectedTokensAfterEnd{})
                         .tokens(id(View{"code"})));
        }(),
        [] {
            return NestTokensData("BlockAfterEnd")
                .in(filter::line().tokens(filter::id(View{"begin"})).insignificants(filter::BlockStartColon{}),
                    filter::line()
                        .insignificants(filter::newLine(), filter::blockEnd(View{"end"}))
                        .tokens(filter::id(View{"more"}))
                        .insignificants(filter::BlockStartColon{}))
                .out(line()
                         .tokens(id(View{"begin"}))
                         .insignificants(filter::BlockStartColon{})
                         .tokens(blk())
                         .insignificants(filter::newLine(), filter::blockEnd(View{"end"}), UnexpectedTokensAfterEnd{})
                         .tokens(id(View{"more"}))
                         .insignificants(filter::BlockStartColon{})
                         .tokens(blk())
                         .insignificants(MissingBlockEnd{}));
        }()),
    [](const ::testing::TestParamInfo<NestTokensData>& inf) { return inf.param.name; });
