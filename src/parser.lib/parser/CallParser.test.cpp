#include "parser/CallParser.h"

#include "parser/Tree.builder.h"
#include "parser/Tree.ostream.h"
#include "parser/TypeTree.ostream.h"

#include "nesting/Token.builder.h"
#include "nesting/Token.ostream.h"

#include "instance/Function.builder.h"
#include "instance/Function.ostream.h"
#include "instance/Scope.builder.h"
#include "instance/Type.builder.h"
#include "instance/TypeTree.builder.h"

#include "diagnostic/Diagnostic.ostream.h"

#include "meta/Optional.ostream.h"

#include <gtest/gtest.h>

#include <map>
#include <sstream>
#include <vector>

using namespace parser;

using diagnostic::Diagnostics;
using ValueNodes = std::map<std::string, OptNode>;
using IndexTyped = std::map<size_t, OptNameTypeValue>;
using FunctionViews = std::vector<instance::FunctionView>;

template<class... Ts>
inline auto operator<<(std::ostream& out, const std::map<Ts...>& map) -> std::ostream& {
    auto second = bool{};
    for (auto& [k, v] : map) {
        out << '[' << k << "]= " << v;
        if (second) out << ", ";
        second = true;
    }
    return out;
}

struct TestCallExternal {
    ValueNodes valueNodes{};
    IndexTyped typed{};
    std::string diagnostics{};

    template<class Type>
    auto intrinsicType(meta::Type<Type>) -> instance::TypeView {
        return {};
    }
    void reportDiagnostic(diagnostic::Diagnostic diagnostic) {
        auto stream = std::stringstream{};
        stream << diagnostics << '\n';
        stream << diagnostic;
        diagnostics = stream.str();
    }

    auto parserForType(const TypeExpression& type) {
        return [this, type](BlockLineView& blv) -> OptNode {
            if (!blv) return {};
            auto k = [&] {
                auto ks = std::stringstream{};
                ks << '[' << blv.index() << ']' << type;
                return ks.str();
            }();
            auto it = valueNodes.find(k);
            ++blv;
            if (it == valueNodes.end()) {
                std::cout << "WARNING: missing valueNode for: " << k << '\n';
                return {};
            }
            return it->second;
        };
    }

    template<class Callback>
    auto parseTypedWithCallback(BlockLineView& blv, Callback&& cb) -> OptNameTypeValue {
        if (!blv) return {};
        auto k = blv.index();
        auto it = typed.find(k);
        if (it == typed.end()) {
            std::cout << "WARNING: missing indexTyped at: " << k << '\n';
            return {};
        }
        auto r = it->second;
        if (r) ++blv;
        cb(r.value());
        return r;
    }
};

struct CallParserData {
    const char* name{};
    std::shared_ptr<Scope> scope{};
    BlockLine input{};
    ValueNodes valueNodes{};
    IndexTyped indexTyped{};
    FunctionViews functions{};
    // expected
    int os_complete{};
    std::string diagnostics{};

    CallParserData(const char* name)
        : name{name}
        , scope{std::make_shared<Scope>()} {}

    template<class... Instance>
    auto ctx(Instance&&... instance) && -> CallParserData {
        instance::buildScope(*scope, std::forward<Instance>(instance)...);
        return std::move(*this);
    }
    template<class... Token>
    auto in(Token&&... token) && -> CallParserData {
        input = BlockLine{{nesting::buildToken(std::forward<Token>(token))...}, {}};
        return std::move(*this);
    }
    template<class Expr>
    auto value(std::string key, Expr&& expr) && -> CallParserData {
        valueNodes[std::move(key)] = std::move(expr).build(*scope);
        return std::move(*this);
    }
    auto typed(size_t key, details::NameTypeValueBuilder typed) && -> CallParserData {
        indexTyped[key] = std::move(typed).build(*scope);
        return std::move(*this);
    }
    template<size_t N>
    auto load(const char (&name)[N]) && -> CallParserData {
        auto range = scope->operator[](strings::View{name});
        for (auto& f : range) functions.push_back(&f.second.get<instance::Function>());
        return std::move(*this);
    }
    auto complete(int count) && -> CallParserData {
        os_complete = count;
        return std::move(*this);
    }
    auto diag(std::string& text) && -> CallParserData {
        diagnostics = text;
        return std::move(*this);
    }
};

static auto operator<<(std::ostream& out, const CallParserData& cpd) -> std::ostream& {
    out << "name: " << cpd.name << "\n";
    out << "input:\n";
    out << cpd.input << '\n';
    out << "valueNodes: " << cpd.valueNodes << '\n';
    out << "indexTyped: " << cpd.indexTyped << '\n';
    out << "expected:\n";
    out << "  complete: " << cpd.os_complete << '\n';
    out << "  diagnostics: " << cpd.diagnostics << '\n';
    return out;
}

class CallParserP : public testing::TestWithParam<CallParserData> {};

TEST_P(CallParserP, parse) {
    const CallParserData& data = GetParam();
    const auto scope = data.scope;

    auto ext = TestCallExternal{};
    ext.typed = data.indexTyped;
    ext.valueNodes = data.valueNodes;

    auto os = CallOverloads{};
    for (auto& fv : data.functions) {
        auto item = CallOverloads::Item{};
        item.function = fv;
        item.active = true;
        os.items.emplace_back(std::move(item));
    }

    auto it = BlockLineView{&data.input};

    parser::CallParser::parse(os, it, ext);

    EXPECT_FALSE(it.hasNext());
    EXPECT_EQ(data.os_complete, std::count_if(begin(os.items), end(os.items), [](auto& o) { return o.complete; }));
    EXPECT_EQ(data.diagnostics, ext.diagnostics);
}

INSTANTIATE_TEST_CASE_P(
    simple,
    CallParserP,
    ::testing::Values( //
        [] {
            return CallParserData("PositionArg") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(
                        instance::param("v").right().type(type().instance("NumLit"))))
                .in(nesting::num("1"))
                .load("print")
                .typed(0, parser::typed())
                .value("[0]:NumLit(bytes: 176)", parser::expr(nesting::num("1")).typeName("NumLit"))
                .complete(1);
        }(),
        [] {
            return CallParserData("NamedArg") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(
                        instance::param("v").right().type(type().instance("NumLit"))))
                .in(nesting::id(View{"v="}), nesting::num("1"))
                .load("print")
                .typed(0, parser::typed("v"))
                .value("[1]:NumLit(bytes: 176)", parser::expr(nesting::num("1")).typeName("NumLit"))
                .complete(1);
        }(),
        [] {
            return CallParserData("Ambigious") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(
                        instance::param("v").right().type(type().instance("NumLit"))),
                    instance::fun("print").runtime().params(
                        instance::param("w").right().type(type().instance("NumLit"))))
                .in(nesting::num("1"))
                .load("print")
                .typed(0, parser::typed())
                .value("[0]:NumLit(bytes: 176)", parser::expr(nesting::num("1")).typeName("NumLit"))
                .complete(2);
        }(),
        [] {
            return CallParserData("ResolveByArgName") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(
                        instance::param("v").right().type(type().instance("NumLit"))),
                    instance::fun("print").runtime().params(
                        instance::param("w").right().type(type().instance("NumLit"))))
                .in(nesting::id(View{"v="}), nesting::num("1"))
                .load("print")
                .typed(0, parser::typed("v"))
                .value("[1]:NumLit(bytes: 176)", parser::expr(nesting::num("1")).typeName("NumLit"))
                .complete(1);
        }(),
        [] {
            return CallParserData("ResolveByArgCount") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(
                        instance::param("v").right().type(type().instance("NumLit")),
                        instance::param("w").right().type(type().instance("NumLit"))),
                    instance::fun("print").runtime().params(
                        instance::param("w").right().type(type().instance("NumLit"))))
                .in(nesting::num("1"))
                .load("print")
                .typed(0, parser::typed())
                .value("[0]:NumLit(bytes: 176)", parser::expr(nesting::num("1")).typeName("NumLit"))
                .complete(1);
        }()),
    [](const ::testing::TestParamInfo<CallParserData>& inf) { return inf.param.name; });
