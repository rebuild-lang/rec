#pragma once
#include "expressionTree.h"

#include "instance/ScopeLookup.h"

namespace parser::expression {

using Scope = instance::Scope;

namespace details {

class ValueBuilder;
using ValueBuilders = std::vector<ValueBuilder>;

using Function = instance::Function;

struct ArgumentBuilder {
    using This = ArgumentBuilder;

    Name name_;
    ValueBuilders values_;

    template<size_t N, class... Value>
    ArgumentBuilder(const char (&name)[N], Value &&... value)
        : name_(name)
        , values_({std::forward<Value>(value)...}) {}

    auto build(const Scope &scope, const Function &fun) && -> ArgumentAssignment;
};
using ArgumentBuilders = std::vector<ArgumentBuilder>;

struct InvokeBuilder {
    using This = InvokeBuilder;

    Name name_;
    ArgumentBuilders left_;
    ArgumentBuilders right_;

    template<size_t N>
    InvokeBuilder(const char (&name)[N])
        : name_(name) {}

    template<class... Args>
    auto right(Args &&... args) && -> This { //
        right_.insert(right_.end(), {std::forward<Args>(args)...});
        return *this;
    }

    auto build(const Scope &scope) && -> Invocation {
        Invocation invocation;
        const auto &fun = instance::lookupA<instance::Function>(scope, name_);
        invocation.function = &fun;
        for (auto &&arg : std::move(left_)) invocation.arguments.emplace_back(std::move(arg).build(scope, fun));
        for (auto &&arg : std::move(right_)) invocation.arguments.emplace_back(std::move(arg).build(scope, fun));
        return invocation;
    }
};

using ValueVariant = meta::Variant<LiteralVariant, InvokeBuilder>;

class ValueBuilder : public ValueVariant {
    using This = ValueBuilder;

public:
    META_VARIANT_CONSTRUCT(ValueBuilder, ValueVariant)

    auto build(const Scope &scope) && -> Node {
        return std::move(*this).visit(
            [](LiteralVariant &&lit) -> Node {
                return Literal{std::move(lit), {}};
            },
            [&](InvokeBuilder &&inv) -> Node { return std::move(inv).build(scope); } //
        );
    }
};

template<class Expr>
struct ExpressionBuilder {
    static auto build(const Scope &, Expr &&expr) -> Node { return std::move(expr); }
};

template<>
struct ExpressionBuilder<InvokeBuilder> {
    static auto build(const Scope &scope, InvokeBuilder &&inv) -> Node { return std::move(inv).build(scope); }
};

inline auto ArgumentBuilder::build(const Scope &scope, const Function &fun) && -> ArgumentAssignment {
    ArgumentAssignment as;
    auto arg = fun.lookupArgument(name_);
    if (!arg) throw "missing argument";
    auto v = arg.value();
    as.argument = &v.get();
    as.values.reserve(values_.size());
    for (auto &&val : std::move(values_)) as.values.emplace_back(std::move(val).build(scope));
    return as;
}

} // namespace details

template<size_t N>
auto invoke(const char (&name)[N]) -> details::InvokeBuilder {
    return {name};
}

template<size_t N, class... Value>
auto arg(const char (&name)[N], Value &&... value) -> details::ArgumentBuilder {
    return {name, std::forward<Value>(value)...};
}

template<class Expr>
auto buildExpression(const Scope &scope, Expr &&expr) -> Node {
    return details::ExpressionBuilder<Expr>::build(scope, std::forward<Expr>(expr));
}

} // namespace parser::expression
