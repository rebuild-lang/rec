#include "parser/Parser.h"

#include "parser/Tree.builder.h"
#include "parser/Tree.ostream.h"
#include "parser/Type.builder.h"

#include "nesting/Token.builder.h"
#include "nesting/Token.ostream.h"

#include "instance/Function.builder.h"
#include "instance/Function.ostream.h"
#include "instance/Scope.builder.h"
#include "instance/Type.builder.h"

#include "gtest/gtest.h"

#include <memory>

using namespace parser;

using instance::Scope;
using nesting::BlockLine;

struct ExpressionParserData {
    const char* name{};
    std::shared_ptr<Scope> scope{};
    BlockLine input{};
    std::shared_ptr<Block> expected{};

    ExpressionParserData(const char* name)
        : name{name}
        , scope{std::make_shared<Scope>()}
        , expected{std::make_shared<Block>()} {}

    template<class... Instance>
    auto ctx(Instance&&... instance) && -> ExpressionParserData {
        instance::buildScope(*scope, std::forward<Instance>(instance)...);
        return std::move(*this);
    }

    template<class... Token>
    auto in(Token&&... token) && -> ExpressionParserData {
        input = BlockLine{{nesting::buildToken(std::forward<Token>(token))...}, {}};
        return std::move(*this);
    }

    template<class... Expr>
    auto out(Expr&&... expr) && -> ExpressionParserData {
        expected->nodes.reserve(expected->nodes.size() + sizeof...(Expr));
        (expected->nodes.emplace_back(parser::buildExpression(*scope, std::forward<Expr>(expr))), ...);
        return std::move(*this);
    }
};

static auto operator<<(std::ostream& out, const ExpressionParserData& epd) -> std::ostream& {
    out << "name: " << epd.name << "\n";
    out << "input:\n";
    out << epd.input << '\n';
    out << "expected:\n";
    out << *epd.expected << '\n';
    return out;
}

class ExpressionParser : public testing::TestWithParam<ExpressionParserData> {};

struct IntrinsicType {
    std::shared_ptr<Scope> scope;

    template<class T>
    auto operator()(meta::Type<T>) -> instance::TypeView {
        if constexpr (std::is_same_v<T, NameTypeValue>) {
            auto& m = (*scope)[strings::View{"Typed"}].frontValue().get<instance::Module>();
            auto& t = m.locals[strings::View{"type"}].frontValue().get<instance::Type>();
            return &t;
        }
        if constexpr (std::is_same_v<T, NumberLiteral>) {
            auto& m = (*scope)[strings::View{"NumLit"}].frontValue().get<instance::Module>();
            auto& t = m.locals[strings::View{"type"}].frontValue().get<instance::Type>();
            return &t;
        }
        return {};
    }
};

TEST_P(ExpressionParser, calls) {
    const ExpressionParserData& data = GetParam();
    const auto input = nesting::BlockLiteral{{}, {{data.input}}};
    const auto scope = data.scope;
    const auto& expected = *data.expected;

    auto context = Context{[scope](const strings::View& id) { return (*scope)[id]; },
                           [=](const parser::Call&) -> OptNode { return {}; },
                           IntrinsicType{scope}};

    auto parsed = parser::Parser::parse(input, context);

    ASSERT_EQ(parsed, expected);
}

INSTANTIATE_TEST_CASE_P(
    simple,
    ExpressionParser,
    ::testing::Values( //
        [] {
            return ExpressionParserData("Call_Number_Literal") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(instance::param("v").right().type(type("NumLit"))))
                .in(nesting::id(View{"print"}), nesting::num("1"))
                .out(parser::call("print").right(arg("v", parser::expr(nesting::num("1")).typeName("NumLit"))));
        }(),
        [] {
            return ExpressionParserData("Call_VarDecl") //
                .ctx( //
                    instance::typeModT<parser::NameTypeValue>("Typed"),
                    instance::typeModT<uint64_t>("u64"),
                    instance::fun("var").runtime().params(instance::param("v").right().type(type("Typed"))))
                .in(nesting::id(View{"var"}), nesting::id(View{"i"}), nesting::colon(), nesting::id(View{"u64"}))
                .out(parser::call("var").right(arg("v", parser::expr(typed("i").type(mod("u64"))).typeName("Typed"))));
        }(),
        [] {
            auto tupleRef = parser::TupleRef{};
            return ExpressionParserData("Tuple_Ref") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::typeModT<parser::NameTypeValue>("Typed"),
                    instance::typeModT<uint64_t>("u64"))
                .in(nesting::bracketOpen(),
                    nesting::id(View{"a"}),
                    nesting::op("="),
                    nesting::num("1"),
                    nesting::comma(),
                    nesting::id(View{"b"}),
                    nesting::op("="),
                    nesting::id(View{"a"}),
                    nesting::bracketClose())
                .out(tuple(
                    typed("a").value(parser::expr(nesting::num("1")).typeName("NumLit")), //
                    tupleRef,
                    typed("b").value(parser::expr(&tupleRef))));
        }()),
    [](const ::testing::TestParamInfo<ExpressionParserData>& inf) { return inf.param.name; });
