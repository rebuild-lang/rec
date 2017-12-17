#pragma once
#include "expression_tree.h"

#include "instance/scope_lookup.h"

namespace parser::expression {

using scope_t = instance::scope_t;

namespace details {

class val_builder;
using val_builder_vec = std::vector<val_builder>;

using function_t = instance::function_t;

struct arg_builder {
    using this_t = arg_builder;

    name_t name_;
    val_builder_vec values_;

    template<size_t N, class... Value>
    arg_builder(const char (&name)[N], Value &&... value)
        : name_(name)
        , values_({std::forward<Value>(value)...}) {}

    auto build(const scope_t &scope, const function_t &fun) && -> argument_assignment;
};
using arg_builder_vec = std::vector<arg_builder>;

struct invoke_builder {
    using this_t = invoke_builder;

    name_t name_;
    arg_builder_vec left_;
    arg_builder_vec right_;

    template<size_t N>
    invoke_builder(const char (&name)[N])
        : name_(name) {}

    template<class... Args>
    auto right(Args &&... args) && -> this_t { //
        right_.insert(right_.end(), {std::forward<Args>(args)...});
        return *this;
    }

    auto build(const scope_t &scope) && -> invocation_t {
        invocation_t invocation;
        const auto &fun = instance::lookup_a<instance::function_t>(scope, name_);
        invocation.function = &fun;
        for (auto &&arg : std::move(left_)) invocation.arguments.emplace_back(std::move(arg).build(scope, fun));
        for (auto &&arg : std::move(right_)) invocation.arguments.emplace_back(std::move(arg).build(scope, fun));
        return invocation;
    }
};

using value_variant = meta::variant<literal_variant, invoke_builder>;

class val_builder : public value_variant {
    using this_t = val_builder;

public:
    META_VARIANT_CONSTRUCT(val_builder, value_variant)

    auto build(const scope_t &scope) && -> node_t {
        return std::move(*this).visit(
            [](literal_variant &&lit) -> node_t {
                return literal_t{std::move(lit), {}};
            },
            [&](invoke_builder &&inv) -> node_t { return std::move(inv).build(scope); } //
        );
    }
};

template<class Expr>
struct expr_builder {
    static auto build(const scope_t &, Expr &&expr) -> node_t { return std::move(expr); }
};

template<>
struct expr_builder<invoke_builder> {
    static auto build(const scope_t &scope, invoke_builder &&inv) -> node_t { return std::move(inv).build(scope); }
};

inline argument_assignment arg_builder::build(const scope_t &scope, const function_t &fun) && {
    argument_assignment as;
    auto arg = fun.lookup_argument(name_);
    if (!arg) throw "missing argument";
    auto v = arg.value();
    as.argument = &v.get();
    as.values.reserve(values_.size());
    for (auto &&val : std::move(values_)) as.values.emplace_back(std::move(val).build(scope));
    return as;
}

} // namespace details

template<size_t N>
auto invoke(const char (&name)[N]) -> details::invoke_builder {
    return {name};
}

template<size_t N, class... Value>
auto arg(const char (&name)[N], Value &&... value) -> details::arg_builder {
    return {name, std::forward<Value>(value)...};
}

template<class Expr>
auto build_expr(const scope_t &scope, Expr &&expr) -> node_t {
    return details::expr_builder<Expr>::build(scope, std::forward<Expr>(expr));
}

} // namespace parser::expression
