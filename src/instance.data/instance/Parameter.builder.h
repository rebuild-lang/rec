#pragma once
#include "Parameter.h"

#include "Scope.h"
#include "ScopeLookup.h"

#include <cassert>
#include <functional>

namespace instance {

namespace details {

using TypeBuilder = std::function<auto(const Scope&)->parser::TypeView>;

struct ParameterBuilder {
    using This = ParameterBuilder;
    ParameterPtr parameter{};
    VariablePtr variable{};
    TypeBuilder typeBuilder{};

    [[nodiscard]] ParameterBuilder()
        : parameter(std::make_shared<Parameter>())
        , variable(std::make_shared<Variable>()) {
        parameter->variable = variable.get();
        variable->parameter = parameter.get();
        variable->flags |= VariableFlag::function_parameter;
    }

    template<size_t N>
    [[nodiscard]] explicit ParameterBuilder(const char (&name)[N])
        : ParameterBuilder() {
        parameter->name = Name{name};
        variable->name = Name{name};
        parameter->side = ParameterSide::right;
    }

    template<class Builder>
    [[nodiscard]] auto type(Builder&& b) && -> ParameterBuilder {
        typeBuilder = [b2 = std::move(b)](const Scope& scope) mutable { return std::move(b2).build(scope); };
        return std::move(*this);
    }

    [[nodiscard]] auto left() && -> ParameterBuilder {
        parameter->side = ParameterSide::left;
        return std::move(*this);
    }
    [[nodiscard]] auto right() && -> ParameterBuilder {
        parameter->side = ParameterSide::right;
        return std::move(*this);
    }
    [[nodiscard]] auto result() && -> ParameterBuilder {
        parameter->side = ParameterSide::result;
        return std::move(*this);
    }
    // auto optional() && -> ParameterBuilder {
    //     parameter->flags |= ParameterFlag::optional;
    //     return std::move(*this);
    // }

    [[nodiscard]] auto build(const Scope& scope, LocalScope& parameterScope) && -> ParameterPtr {
        if (typeBuilder) {
            variable->type = typeBuilder(scope);
        }
        parameterScope.emplace(std::move(variable));
        return std::move(parameter);
    }
};

} // namespace details

template<size_t N>
auto param(const char (&name)[N]) {
    return details::ParameterBuilder{name};
}

} // namespace instance
