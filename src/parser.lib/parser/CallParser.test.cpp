#include "parser/CallParser.h"

#include "parser/Expression.builder.h"
#include "parser/Expression.ostream.h"
#include "parser/Type.builder.h"
#include "parser/Type.ostream.h"

#include "nesting/Token.builder.h"
#include "nesting/Token.ostream.h"

#include "instance/Function.builder.h"
#include "instance/Function.ostream.h"
#include "instance/Scope.builder.h"
#include "instance/Type.builder.h"

#include "diagnostic/Diagnostic.ostream.h"

#include "meta/Optional.ostream.h"

#include <gtest/gtest.h>

#include <map>
#include <sstream>
#include <vector>

using namespace parser;

using diagnostic::Diagnostics;
using instance::ScopePtr;
using ValueNodes = std::map<std::string, OptValueExpr>;
using IndexNtvs = std::map<size_t, OptNameTypeValue>;
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
    IndexNtvs indexNtvs{};
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

    auto parserForType(const TypeView& type) {
        return [this, type](BlockLineView& blv) -> OptValueExpr {
            if (!blv) return {};
            auto k = [&] {
                auto ks = std::stringstream{};
                ks << '[' << blv.index() << ']' << *type;
                auto s = ks.str();
                auto p = std::string("(bytes:");
                auto it = std::search(begin(s), end(s), begin(p), end(p));
                return std::string(s.begin(), it);
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
    auto parseNtvWithCallback(BlockLineView& blv, Callback&& cb) -> OptNameTypeValue {
        if (!blv) return {};
        auto k = blv.index();
        auto it = indexNtvs.find(k);
        if (it == indexNtvs.end()) {
            std::cout << "WARNING: missing indexNtv at: " << k << '\n';
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
    ScopePtr scope{};
    BlockLine input{};
    ValueNodes valueNodes{};
    IndexNtvs indexNtvs{};
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
    auto indexNtv(size_t key, details::NameTypeValueBuilder builder) && -> CallParserData {
        indexNtvs[key] = std::move(builder).build(*scope);
        return std::move(*this);
    }
    template<size_t N>
    auto load(const char (&name)[N]) && -> CallParserData {
        auto range = scope->byName(strings::View{name});
        for (auto& f : range) functions.push_back(f.get<instance::FunctionPtr>().get());
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
    out << "indexNtv: " << cpd.indexNtvs << '\n';
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
    ext.indexNtvs = data.indexNtvs;
    ext.valueNodes = data.valueNodes;

    auto os = CallOverloads{};
    for (auto& fv : data.functions) {
        os.items.emplace_back(fv);
    }

    auto it = BlockLineView{&data.input};

    parser::CallParser::parse(os, it, ext);

    EXPECT_FALSE(it.hasNext());
    EXPECT_EQ(data.os_complete, std::count_if(begin(os.items), end(os.items), [](auto& o) { return o.complete; }));
    EXPECT_EQ(data.diagnostics, ext.diagnostics);
}

INSTANTIATE_TEST_SUITE_P(
    simple,
    CallParserP,
    ::testing::Values( //
        [] {
            return CallParserData("PositionArg") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(instance::param("v").right().type(type("NumLit"))))
                .in(nesting::num("1"))
                .load("print")
                .indexNtv(0, parser::ntv())
                .value("[0]:NumLit", parser::valueExpr(nesting::num("1")).typeName("NumLit"))
                .complete(1);
        }(),
        [] {
            return CallParserData("NamedArg") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(instance::param("v").right().type(type("NumLit"))))
                .in(nesting::id(View{"v="}), nesting::num("1"))
                .load("print")
                .indexNtv(0, parser::ntv("v"))
                .value("[1]:NumLit", parser::valueExpr(nesting::num("1")).typeName("NumLit"))
                .complete(1);
        }(),
        [] {
            return CallParserData("Ambigious") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(instance::param("v").right().type(type("NumLit"))),
                    instance::fun("print").runtime().params(instance::param("w").right().type(type("NumLit"))))
                .in(nesting::num("1"))
                .load("print")
                .indexNtv(0, parser::ntv())
                .value("[0]:NumLit", parser::valueExpr(nesting::num("1")).typeName("NumLit"))
                .complete(2);
        }(),
        [] {
            return CallParserData("ResolveByArgName") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(instance::param("v").right().type(type("NumLit"))),
                    instance::fun("print").runtime().params(instance::param("w").right().type(type("NumLit"))))
                .in(nesting::id(View{"v="}), nesting::num("1"))
                .load("print")
                .indexNtv(0, parser::ntv("v"))
                .value("[1]:NumLit", parser::valueExpr(nesting::num("1")).typeName("NumLit"))
                .complete(1);
        }(),
        [] {
            return CallParserData("ResolveByArgCount") //
                .ctx( //
                    instance::typeModT<nesting::NumberLiteral>("NumLit"),
                    instance::fun("print").runtime().params(
                        instance::param("v").right().type(type("NumLit")),
                        instance::param("w").right().type(type("NumLit"))),
                    instance::fun("print").runtime().params(instance::param("w").right().type(type("NumLit"))))
                .in(nesting::num("1"))
                .load("print")
                .indexNtv(0, parser::ntv())
                .value("[0]:NumLit", parser::valueExpr(nesting::num("1")).typeName("NumLit"))
                .complete(1);
        }()),
    [](const ::testing::TestParamInfo<CallParserData>& inf) { return inf.param.name; });
