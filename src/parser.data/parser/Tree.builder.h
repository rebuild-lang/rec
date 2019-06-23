#pragma once
#include "Tree.h"

#include "instance/ScopeLookup.h"
#include "instance/TypeTree.builder.h"

namespace parser {

using instance::Scope;

namespace details {

class ValueExprBuilder;
using ValueExprBuilders = std::vector<ValueExprBuilder>;
using ValueExprBuilderPtr = std::shared_ptr<ValueExprBuilder>;

using instance::Function;

struct ArgumentBuilder {
    using This = ArgumentBuilder;

    View name{};
    ValueExprBuilders values{};

    template<size_t N, class... Value>
    ArgumentBuilder(const char (&name)[N], Value&&... value)
        : name(name) {
        values.reserve(sizeof...(Value));
        (values.push_back(std::forward<Value>(value)), ...);
    }

    auto build(const Scope& scope, const Function& fun) && -> ArgumentAssignment;
};
using ArgumentBuilders = std::vector<ArgumentBuilder>;

struct CallBuilder {
    using This = CallBuilder;

    View name{};
    ArgumentBuilders leftBuilder{};
    ArgumentBuilders rightBuilder{};

    template<size_t N>
    CallBuilder(const char (&name)[N])
        : name(name) {}

    template<class... Args>
    auto right(Args&&... args) && -> This { //
        (rightBuilder.insert(rightBuilder.end(), std::forward<Args>(args)), ...);
        return std::move(*this);
    }

    auto build(const Scope& scope) && -> Call {
        Call call;
        const auto& fun = instance::lookupA<instance::Function>(scope, name);
        call.function = &fun;
        for (auto&& arg : std::move(leftBuilder)) call.arguments.emplace_back(std::move(arg).build(scope, fun));
        for (auto&& arg : std::move(rightBuilder)) call.arguments.emplace_back(std::move(arg).build(scope, fun));
        return call;
    }
};

struct NameTypeValueBuilder {
    using This = NameTypeValueBuilder;

    Name name{};
    TypeExprBuilder typeExpr{};
    ValueExprBuilderPtr valuePtr{};

    NameTypeValueBuilder() = default;

    template<size_t N>
    NameTypeValueBuilder(const char (&name)[N])
        : name(name) {}

    auto type(TypeExprBuilder&& builder) && -> This {
        typeExpr = std::move(builder);
        return std::move(*this);
    }

    auto value(ValueExprBuilder&& value) && -> This;

    auto build(const Scope& scope) && -> NameTypeValue;
};

using ValueExprBuilderVariant = meta::Variant<
    nesting::StringLiteral,
    nesting::NumberLiteral,
    nesting::OperatorLiteral,
    nesting::IdentifierLiteral,
    nesting::BlockLiteral,
    CallBuilder,
    NameTypeValueBuilder>;

class ValueExprBuilder : public ValueExprBuilderVariant {
    using This = ValueExprBuilder;
    Name m_typeName{};

public:
    ValueExprBuilder(const This&) = default;
    ValueExprBuilder(This&&) = default;
    ValueExprBuilder& operator=(const This&) = default;
    ValueExprBuilder& operator=(This&&) = default;

    META_VARIANT_CONSTRUCT(ValueExprBuilder, ValueExprBuilderVariant)

    template<size_t N>
    auto typeName(const char (&tn)[N]) && -> This {
        m_typeName = Name{tn};
        return std::move(*this);
    }

    auto build(const Scope& scope) && -> Node {
        return std::move(*this).visit(
            [&](CallBuilder&& inv) -> Node { return std::move(inv).build(scope); }, //
            [&](NameTypeValueBuilder&& typ) -> Node {
                auto& type = instance::lookupA<instance::Type>(scope, m_typeName);
                return Value{std::move(typ).build(scope), TypeExpression{TypeInstance{&type}}};
            },
            [&](auto&& lit) -> Node {
                auto& type = instance::lookupA<instance::Type>(scope, m_typeName);
                return Value{std::move(lit), TypeExpression{TypeInstance{&type}}};
            });
    }
};

template<class Expr>
struct ExpressionBuilder {
    static auto build(const Scope&, Expr&& expr) -> Node { return std::move(expr); }
};

template<>
struct ExpressionBuilder<CallBuilder> {
    static auto build(const Scope& scope, CallBuilder&& inv) -> Node { return std::move(inv).build(scope); }
};

inline auto ArgumentBuilder::build(const Scope& scope, const Function& fun) && -> ArgumentAssignment {
    auto as = ArgumentAssignment{};
    auto param = fun.lookupParameter(name);
    if (!param) throw "missing argument";
    auto* v = param.value();
    as.parameter = v;
    as.values.reserve(values.size());
    for (auto&& val : std::move(values)) as.values.emplace_back(std::move(val).build(scope));
    return as;
}

inline auto NameTypeValueBuilder::value(ValueExprBuilder&& value) && -> This {
    valuePtr.reset(new ValueExprBuilder{std::move(value)});
    return std::move(*this);
}

inline auto NameTypeValueBuilder::build(const Scope& scope) && -> NameTypeValue {
    auto r = NameTypeValue{};
    if (!name.isEmpty()) r.name = name;
    if (typeExpr) r.type = std::move(typeExpr).build(scope);
    if (valuePtr) r.value = std::move(*valuePtr).build(scope);
    return r;
}

} // namespace details

template<size_t N>
auto call(const char (&name)[N]) -> details::CallBuilder {
    return {name};
}

template<size_t N>
auto typed(const char (&name)[N]) -> details::NameTypeValueBuilder {
    return {name};
}
inline auto typed() -> details::NameTypeValueBuilder { return {}; }

template<size_t N, class... Value>
auto arg(const char (&name)[N], Value&&... value) -> details::ArgumentBuilder {
    return {name, std::forward<Value>(value)...};
}

template<class Expr>
auto expr(Expr&& expr) -> details::ValueExprBuilder {
    return {std::forward<Expr>(expr)};
}

template<class Expr>
auto buildExpression(const Scope& scope, Expr&& expr) -> Node {
    return details::ExpressionBuilder<Expr>::build(scope, std::forward<Expr>(expr));
}

} // namespace parser
