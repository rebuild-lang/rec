#pragma once
#include "parser/expression/TypeTree.h"

#include "ScopeLookup.h"
#include "Type.h"

namespace parser::expression {

namespace details {

struct TypeExprInstanceBuilder {
    Name name{};

    TypeExprInstanceBuilder() = default;

    template<size_t N>
    TypeExprInstanceBuilder(const char (&name)[N])
        : name(Name{name}) {}

    auto build(const instance::Scope& scope) && -> TypeExpression {
        auto* t = &instance::lookupA<instance::Type>(scope, name);
        return TypeInstance{t};
    }
};

struct TypeExprArrayBuilder {
    size_t count{};

    auto build(TypeExpression&& expr) && -> TypeExpression {
        return Array{count, std::make_shared<TypeExpression>(std::move(expr))};
    }
};

struct TypeExprPointerBuilder {
    auto build(TypeExpression&& expr) && -> TypeExpression {
        return Pointer{std::make_shared<TypeExpression>(std::move(expr))};
    }
};

struct TypeExprBuilder {
    using This = TypeExprBuilder;
    using Nodes = meta::Variant<TypeExprArrayBuilder, TypeExprPointerBuilder>;

    TypeExprInstanceBuilder terminal{};
    std::vector<Nodes> nodes;

    template<size_t N>
    auto instance(const char (&name)[N]) && {
        terminal = TypeExprInstanceBuilder{name};
        return *this;
    }

    auto pointer() && -> This {
        if (not terminal.name.isEmpty()) throw "terminal present";
        nodes.push_back(TypeExprPointerBuilder{});
        return *this;
    }

    auto array(size_t count) && -> This {
        if (not terminal.name.isEmpty()) throw "terminal present";
        nodes.push_back(TypeExprArrayBuilder{count});
        return *this;
    }

    auto build(const instance::Scope& scope) && -> TypeExpression {
        auto c = std::move(terminal).build(scope);
        for (auto& node : nodes) {
            c = std::move(node).visit([&](auto&& b) { return std::move(b).build(std::move(c)); });
        }
        return c;
    }
};

} // namespace details

inline auto type() -> details::TypeExprBuilder { return {}; }

} // namespace parser::expression
