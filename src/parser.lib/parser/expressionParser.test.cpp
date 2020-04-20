#include "parser/Parser.h"

#include "parser/Expression.builder.h"
#include "parser/Expression.ostream.h"
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
using instance::ScopePtr;
using nesting::BlockLine;

struct ExpressionParserData {
    const char* name{};
    ScopePtr scope{};
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
        expected->expressions.reserve(expected->expressions.size() + sizeof...(Expr));
        (expected->expressions.emplace_back(parser::buildBlockExpr(*scope, std::forward<Expr>(expr))), ...);
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
    ScopePtr scope;

    template<class T>
    auto operator()(meta::Type<T>) -> instance::TypeView {
        if constexpr (std::is_same_v<T, NameTypeValue>) {
            auto& m = scope->byName(strings::View{"NameTypeValue"}).frontValue().get<instance::ModulePtr>();
            return m->locals.byName(strings::View{"type"}).frontValue().get<instance::TypePtr>().get();
        }
        if constexpr (std::is_same_v<T, NumberLiteral>) {
            auto& m = scope->byName(strings::View{"NumLit"}).frontValue().get<instance::ModulePtr>();
            return m->locals.byName(strings::View{"type"}).frontValue().get<instance::TypePtr>().get();
        }
        return {};
    }
};

TEST_P(ExpressionParser, calls) {
    const ExpressionParserData& data = GetParam();
    const auto input = nesting::BlockLiteral{{}, {{data.input}}};
    const auto scope = data.scope;
    const auto& expected = *data.expected;

    auto context = ComposeContext{
        [scope = scope.get()](strings::View id) { return scope->byName(id); },
        [](const parser::Call&) -> OptValueExpr { return {}; },
        IntrinsicType{scope} //
    };

    auto parsed = parser::Parser::parseBlock(input, context);

    ASSERT_EQ(parsed, expected);
}

INSTANTIATE_TEST_CASE_P(
    simple,
    ExpressionParser,
    ::testing::
        Values( //
            [] {
                return ExpressionParserData("Call_Number_Literal") //
                    .ctx( //
                        instance::typeModT<nesting::NumberLiteral>("NumLit"),
                        instance::fun("print").runtime().params(instance::param("v").right().type(type("NumLit"))))
                    .in(nesting::id(View{"print"}), nesting::num("1"))
                    .out(
                        parser::call("print").right(arg("v", parser::valueExpr(nesting::num("1")).typeName("NumLit"))));
            }(),
            [] {
                return ExpressionParserData("Call_VarDecl") //
                    .ctx( //
                        instance::typeModT<parser::NameTypeValue>("NameTypeValue"),
                        instance::typeModT<uint64_t>("u64"),
                        instance::fun("var").runtime().params(instance::param("v").right().type(type("NameTypeValue"))))
                    .in(nesting::id(View{"var"}), nesting::id(View{"i"}), nesting::colon(), nesting::id(View{"u64"}))
                    .out(parser::call("var").right(
                        arg("v", parser::valueExpr(ntv("i").type(typeExpr(type("u64")))).typeName("NameTypeValue"))));
            }(),
            [] {
                auto tupleRef = parser::TypeNameValueRef{};
                return ExpressionParserData("Tuple_Ref") //
                    .ctx( //
                        instance::typeModT<parser::NumberLiteral>("NumLit"),
                        instance::typeModT<parser::NameTypeValue>("NameTypeValue"),
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
                        ntv("a").value(parser::valueExpr(nesting::num("1")).typeName("NumLit")), //
                        tupleRef,
                        ntv("b").value(parser::valueExpr(&tupleRef))));
            }()),
    [](const ::testing::TestParamInfo<ExpressionParserData>& inf) { return inf.param.name; });
