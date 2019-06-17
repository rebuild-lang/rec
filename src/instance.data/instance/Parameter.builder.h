#pragma once
#include "Parameter.h"

#include "Scope.h"
#include "ScopeLookup.h"

#include <cassert>
#include <functional>

namespace instance {

namespace details {

using TypeExprBuilder = std::function<auto(const Scope&)->parser::TypeExpression>;

struct ParameterBuilder {
    using This = ParameterBuilder;
    Parameter arg{};
    TypeExprBuilder typeExprBuilder{};

    template<size_t N>
    explicit ParameterBuilder(const char (&name)[N]) {
        arg.typed.name = Name{name};
    }

    template<class Builder>
    auto type(Builder&& b) -> ParameterBuilder {
        typeExprBuilder = [b2 = std::move(b)](const Scope& scope) mutable { return std::move(b2).build(scope); };
        return *this;
    }

    auto left() -> ParameterBuilder {
        arg.side = ParameterSide::left;
        return *this;
    }
    auto right() -> ParameterBuilder {
        arg.side = ParameterSide::right;
        return *this;
    }
    auto result() -> ParameterBuilder {
        arg.side = ParameterSide::result;
        return *this;
    }
    auto optional() -> ParameterBuilder {
        arg.flags |= ParameterFlag::optional;
        return *this;
    }

    auto build(const Scope& scope, LocalScope& funScope) && -> ParameterView {
        if (typeExprBuilder) {
            arg.typed.type = typeExprBuilder(scope);
        }
        auto node = funScope.emplace(std::move(arg));
        return &node->get<Parameter>();
    }
};

} // namespace details

template<size_t N>
auto param(const char (&name)[N]) {
    return details::ParameterBuilder{name};
}

} // namespace instance
