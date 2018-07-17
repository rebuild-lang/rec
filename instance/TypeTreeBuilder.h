#pragma once
#include "parser/expression/TypeTree.h"

#include "ScopeLookup.h"
#include "Type.h"

namespace parser::expression {

namespace details {

struct TypeExprInstanceBuilder {
    Name name;

    template<size_t N>
    TypeExprInstanceBuilder(const char (&name)[N])
        : name(Name{name}) {}

    auto build(const instance::Scope& scope) && -> TypeExpression {
        auto* t = &instance::lookupA<instance::Type>(scope, name);
        return TypeInstance{t};
    }
};

struct TypeExprPointerBuilder {

    template<size_t N>
    auto instance(const char (&name)[N]) && {
        struct B {
            TypeExprInstanceBuilder instance;

            auto build(const instance::Scope& scope) && -> TypeExpression {
                return Pointer{std::make_shared<TypeExpression>(std::move(instance).build(scope))};
            }
        };
        return B{{name}};
    }
};

struct TypeExprBuilder {

    template<size_t N>
    auto instance(const char (&name)[N]) && {
        return TypeExprInstanceBuilder{name};
    }

    auto pointer() && { return TypeExprPointerBuilder{}; }
};

} // namespace details

inline auto type() -> details::TypeExprBuilder { return {}; }

} // namespace parser::expression
