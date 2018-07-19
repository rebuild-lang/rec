#include "Parser.h"
#include "TreeBuilder.h"
#include "TreeOutput.h"

#include "parser/block/TokenBuilder.h"
#include "parser/block/TokenOutput.h"

#include "instance/FunctionBuilder.h"
#include "instance/FunctionOutput.h"
#include "instance/ScopeBuilder.h"
#include "instance/TypeBuilder.h"
#include "instance/TypeTreeBuilder.h"

#include "gtest/gtest.h"

#include <memory>

using namespace parser;
using namespace parser::expression;

using Scope = instance::Scope;
using BlockLine = block::TokenLine;

struct ExpressionParserData {
    const char* name{};
    std::shared_ptr<Scope> scope{};
    BlockLine input{};
    Block expected{};

    ExpressionParserData(const char* name)
        : name{name}
        , scope{std::make_shared<Scope>()} {}

    template<class... Instance>
    auto ctx(Instance&&... instance) && -> ExpressionParserData {
        instance::buildScope(*scope, std::forward<Instance>(instance)...);
        return std::move(*this);
    }

    template<class... Token>
    auto in(Token&&... token) && -> ExpressionParserData {
        input = BlockLine{block::buildToken(std::forward<Token>(token))...};
        return std::move(*this);
    }

    template<class... Expr>
    auto out(Expr&&... expr) && -> ExpressionParserData {
        expected.nodes.reserve(expected.nodes.size() + sizeof...(Expr));
        auto x = {
            (expected.nodes.emplace_back(expression::buildExpression(*scope, std::forward<Expr>(expr))), 0)... //
        };
        (void)x;
        return std::move(*this);
    }
};

static auto operator<<(std::ostream& out, const ExpressionParserData& epd) -> std::ostream& {
    out << "name: " << epd.name << "\n";
    out << "input:\n";
    out << epd.input << '\n';
    out << "expected:\n";
    out << epd.expected << '\n';
    return out;
}

class ExpressionParser : public testing::TestWithParam<ExpressionParserData> {};

struct IntrinsicType {
    std::shared_ptr<Scope> scope;

    template<class T>
    auto operator()(meta::Type<T>) -> instance::TypeView {
        if constexpr (std::is_same_v<T, Typed>) {
            auto& m = (*scope)[strings::View{"Typed"}]->get<instance::Module>();
            auto& t = (m.locals[strings::View{"type"}])->get<instance::Type>();
            return &t;
        }
        return {};
    }
};

TEST_P(ExpressionParser, calls) {
    const ExpressionParserData& data = GetParam();
    const auto input = block::BlockLiteral{{{data.input}}, {}};
    const auto scope = data.scope;
    const auto& expected = data.expected;

    auto context = Context{[scope](const strings::View& id) { return (*scope)[id]; },
                           [=](const expression::Call&) -> OptNode { return {}; },
                           IntrinsicType{scope}};

    auto parsed = expression::Parser::parse(input, context);

    ASSERT_EQ(parsed, expected);
}

INSTANTIATE_TEST_CASE_P(
    simple,
    ExpressionParser,
    ::testing::Values( //
        ExpressionParserData("Call Number Literal") //
            .ctx( //
                instance::typeMod("NumLit").size(sizeof(void*)).parser(instance::Parser::SingleToken),
                instance::fun("print").runtime().args(
                    instance::arg("v").right().type(type().pointer().instance("NumLit"))))
            .in(block::id("print"), block::num("1"))
            .out(expression::call("print").right(arg("v", block::num("1")))), //
        ExpressionParserData("Call VarDecl") //
            .ctx( //
                instance::typeMod("Typed").size(sizeof(void*)).parser(instance::Parser::IdTypeValue),
                instance::typeMod("u64").size(sizeof(uint64_t)),
                instance::fun("var").runtime().args(
                    instance::arg("v").right().type(type().pointer().instance("Typed"))))
            .in(block::id("var"), block::id("i"), block::colon(), block::id("u64"))
            .out(expression::call("var").right(arg("v", typed("i").type(type().instance("u64"))))) //
        ));
