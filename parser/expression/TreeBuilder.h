#pragma once
#include "Tree.h"

#include "instance/ScopeLookup.h"

namespace parser::expression {

using Scope = instance::Scope;

namespace details {

class ValueBuilder;
using ValueBuilders = std::vector<ValueBuilder>;

using Function = instance::Function;

struct ArgumentBuilder {
    using This = ArgumentBuilder;

    View name{};
    ValueBuilders values{};

    template<size_t N, class... Value>
    ArgumentBuilder(const char (&name)[N], Value&&... value)
        : name(name)
        , values({std::forward<Value>(value)...}) {}

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
        rightBuilder.insert(rightBuilder.end(), {std::forward<Args>(args)...});
        return *this;
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

using ValueVariant = meta::Variant<
    block::StringLiteral,
    block::NumberLiteral,
    block::OperatorLiteral,
    block::IdentifierLiteral,
    block::BlockLiteral,
    CallBuilder>;

class ValueBuilder : public ValueVariant {
    using This = ValueBuilder;

public:
    META_VARIANT_CONSTRUCT(ValueBuilder, ValueVariant)

    auto build(const Scope& scope) && -> Node {
        return std::move(*this).visit(
            [&](CallBuilder&& inv) -> Node { return std::move(inv).build(scope); }, //
            [](auto&& lit) -> Node {
                return TokenLiteral<std::decay_t<decltype(lit)>>{std::move(lit), {}};
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
    auto arg = fun.lookupArgument(name);
    if (!arg) throw "missing argument";
    auto v = arg.value();
    as.argument = &v.get();
    as.values.reserve(values.size());
    for (auto&& val : std::move(values)) as.values.emplace_back(std::move(val).build(scope));
    return as;
}

} // namespace details

template<size_t N>
auto call(const char (&name)[N]) -> details::CallBuilder {
    return {name};
}

template<size_t N, class... Value>
auto arg(const char (&name)[N], Value&&... value) -> details::ArgumentBuilder {
    return {name, std::forward<Value>(value)...};
}

template<class Expr>
auto buildExpression(const Scope& scope, Expr&& expr) -> Node {
    return details::ExpressionBuilder<Expr>::build(scope, std::forward<Expr>(expr));
}

} // namespace parser::expression
