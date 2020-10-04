#pragma once
#include "Type.builder.h"

#include "Expression.h"

#ifdef VALUE_DEBUG_DATA
#    include "parser/Expression.ostream.h"
#endif

#include "instance/ScopeLookup.h"

namespace parser {

using instance::Scope;

struct TypeNameValueRef {
    NameTypeValueView ref{};
};

namespace details {

struct ModuleBuilder {
    Name name{};

    ModuleBuilder() = default;

    template<size_t N>
    ModuleBuilder(const char (&name)[N])
        : name(Name{name}) {}

    operator bool() const { return !name.isEmpty(); }

    auto build(const instance::Scope& scope) && -> ModuleView {
        auto* t = instance::lookupA<instance::ModulePtr>(scope, name).get();
        return t;
    }
};

struct ValueExprBuilder;
using ValueExprBuilders = std::vector<ValueExprBuilder>;
using ValueExprBuilderPtr = std::shared_ptr<ValueExprBuilder>;

struct TypeExprBuilder;
using TypeExprBuilders = std::vector<TypeExprBuilder>;
using TypeExprBuilderPtr = std::shared_ptr<TypeExprBuilder>;

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

struct NameTypeValueBuilder;
using NameTypeValueBuilders = std::vector<NameTypeValueBuilder>;

struct TypeNameValueTupleBuilder {
    using This = TypeNameValueTupleBuilder;

    NameTypeValueBuilders builders{};
    std::vector<NameTypeValueView*> references{};

    template<class... Ts>
    TypeNameValueTupleBuilder(Ts&&... ts) {
        builders.reserve(sizeof...(Ts));
        references.resize(sizeof...(Ts));

        (add(std::forward<Ts>(ts)), ...);
    }

    auto build(const Scope& scope) && -> ValueExpr;

private:
    auto add(NameTypeValueBuilder&& x) { builders.push_back(std::move(x)); }
    auto add(TypeNameValueRef& ref) {
        references[builders.size() - 1] = &ref.ref; //
    }
};

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
        auto fun = instance::lookupA<instance::FunctionPtr>(scope, name);
        call.function = fun.get();
        for (auto&& arg : std::move(leftBuilder)) call.arguments.emplace_back(std::move(arg).build(scope, *fun));
        for (auto&& arg : std::move(rightBuilder)) call.arguments.emplace_back(std::move(arg).build(scope, *fun));
        return call;
    }
};

struct NameTypeValueBuilder {
    using This = NameTypeValueBuilder;

    Name name{};
    TypeExprBuilderPtr typePtr{};
    ValueExprBuilderPtr valuePtr{};

    NameTypeValueBuilder() = default;

    template<size_t N>
    NameTypeValueBuilder(const char (&name)[N])
        : name(name) {}

    auto type(TypeExprBuilder&&) && -> This;
    auto value(ValueExprBuilder&&) && -> This;

    auto build(const Scope&) && -> NameTypeValue;
};

using ValueExprBuilderVariant = meta::Variant<
    nesting::StringLiteral,
    nesting::NumberLiteral,
    nesting::IdentifierLiteral,
    nesting::BlockLiteral,
    CallBuilder,
    NameTypeValueBuilder,
    ModuleBuilder,
    TypeNameValueRef*>;

struct ValueExprBuilder final : ValueExprBuilderVariant {
    using This = ValueExprBuilder;
    Name m_typeName{};

public:
    META_VARIANT_CONSTRUCT(ValueExprBuilder, ValueExprBuilderVariant)

    template<size_t N>
    auto typeName(const char (&tn)[N]) && -> This {
        m_typeName = Name{tn};
        return std::move(*this);
    }

    auto build(const Scope& scope) && -> ValueExpr {
        return std::move(*this).visit(
            [&](CallBuilder&& call) -> ValueExpr { return std::move(call).build(scope); }, //
            [&](NameTypeValueBuilder&& ntv) -> ValueExpr {
                auto type = instance::lookupA<instance::TypePtr>(scope, m_typeName);
                auto value = Value(type.get());
                value.set<NameTypeValue>() = std::move(ntv).build(scope);
                return value;
            },
            [&](ModuleBuilder&& mod) -> ValueExpr { return ModuleReference{std::move(mod).build(scope)}; },
            [&](TypeNameValueRef* ref) -> ValueExpr {
                return NameTypeValueReference{ref->ref}; //
            },
            [&](auto&& lit) -> ValueExpr {
                using Lit = std::remove_const_t<std::remove_reference_t<decltype(lit)>>;
                auto type = instance::lookupA<instance::TypePtr>(scope, m_typeName);
                auto value = Value(type.get());
                value.set<Lit>() = std::move(lit);
                return value;
            });
    }
};

using TypeExprBuilderVariant = meta::Variant<
    nesting::StringLiteral,
    nesting::NumberLiteral,
    nesting::IdentifierLiteral,
    nesting::BlockLiteral,
    CallBuilder,
    TypeBuilder,
    TypeNameValueRef*>;

struct TypeExprBuilder final : TypeExprBuilderVariant {
    using This = TypeExprBuilder;
    Name m_typeName{};

public:
    META_VARIANT_CONSTRUCT(TypeExprBuilder, TypeExprBuilderVariant)

    template<size_t N>
    auto typeName(const char (&tn)[N]) && -> This {
        m_typeName = Name{tn};
        return std::move(*this);
    }

    auto build(const Scope& scope) && -> TypeExpr {
        return std::move(*this).visit(
            [&](CallBuilder&& inv) -> TypeExpr { return std::move(inv).build(scope); }, //
            [&](TypeNameValueRef* ref) -> TypeExpr {
                return NameTypeValueReference{ref->ref}; //
            },
            [&](TypeBuilder&& tb) -> TypeExpr { return TypeReference{std::move(tb).build(scope)}; },
            [&](auto&& lit) -> TypeExpr {
                using Lit = std::remove_const_t<std::remove_reference_t<decltype(lit)>>;
                auto type = instance::lookupA<instance::TypePtr>(scope, m_typeName);
                auto value = Value(type.get());
                value.set<Lit>() = std::move(lit);
                return value;
            });
    }
};

template<class Expr>
struct GenericValueExprBuilder {
    static auto build(const Scope&, Expr&& expr) -> ValueExpr { return std::move(expr); }
};

template<>
struct GenericValueExprBuilder<CallBuilder> {
    static auto build(const Scope& scope, CallBuilder&& inv) -> ValueExpr { return std::move(inv).build(scope); }
};

template<>
struct GenericValueExprBuilder<TypeNameValueTupleBuilder> {
    static auto build(const Scope& scope, TypeNameValueTupleBuilder&& inv) -> ValueExpr {
        return std::move(inv).build(scope);
    }
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

inline auto TypeNameValueTupleBuilder::build(const Scope& scope) && -> ValueExpr {
    auto tuple = NameTypeValueTuple{};
    auto i = 0;
    for (auto&& ts : std::move(builders)) {
        tuple.tuple.emplace_back(std::move(ts).build(scope));
        if (references[i]) //
            *(references[i]) = &tuple.tuple.back();
        i++;
    }
    return tuple;
}

inline auto NameTypeValueBuilder::type(TypeExprBuilder&& value) && -> This {
    typePtr.reset(new TypeExprBuilder{std::move(value)});
    return std::move(*this);
}
inline auto NameTypeValueBuilder::value(ValueExprBuilder&& value) && -> This {
    valuePtr.reset(new ValueExprBuilder{std::move(value)});
    return std::move(*this);
}

inline auto NameTypeValueBuilder::build(const Scope& scope) && -> NameTypeValue {
    auto r = NameTypeValue{};
    if (!name.isEmpty()) r.name = name;
    if (typePtr) r.type = std::move(*typePtr).build(scope);
    if (valuePtr) r.value = std::move(*valuePtr).build(scope);
    return r;
}

} // namespace details

template<size_t N>
auto call(const char (&name)[N]) -> details::CallBuilder {
    return {name};
}

template<size_t N>
auto ntv(const char (&name)[N]) -> details::NameTypeValueBuilder {
    return {name};
}
inline auto ntv() -> details::NameTypeValueBuilder { return {}; }

template<size_t N>
inline auto mod(const char (&name)[N]) -> details::ModuleBuilder {
    return {name};
}

template<size_t N, class... Value>
auto arg(const char (&name)[N], Value&&... value) -> details::ArgumentBuilder {
    return {name, std::forward<Value>(value)...};
}

template<class... Ts>
auto tuple(Ts&&... ts) -> details::TypeNameValueTupleBuilder {
    return {std::forward<Ts>(ts)...};
}

template<class Expr>
auto valueExpr(Expr&& expr) -> details::ValueExprBuilder {
    return {std::forward<Expr>(expr)};
}
template<class Expr>
auto typeExpr(Expr&& expr) -> details::TypeExprBuilder {
    return {std::forward<Expr>(expr)};
}

template<class Expr>
auto buildValueExpr(const Scope& scope, Expr&& expr) -> ValueExpr {
    return details::GenericValueExprBuilder<Expr>::build(scope, std::forward<Expr>(expr));
}

template<class Expr>
auto buildBlockExpr(const Scope& scope, Expr&& expr) -> BlockExpr {
    return buildValueExpr(scope, std::forward<Expr>(expr)).visit([](auto&& expr) -> BlockExpr { return expr; });
}

} // namespace parser
