#pragma once
#include "Argument.h"

#include "Scope.h"
#include "ScopeLookup.h"

#include "functional"

namespace instance {

namespace details {

using TypeExprBuilder = std::function<auto(const Scope&)->parser::TypeExpression>;

struct ArgumentBuilder {
    using This = ArgumentBuilder;
    Argument arg{};
    TypeExprBuilder typeExprBuilder{};

    template<size_t N>
    explicit ArgumentBuilder(const char (&name)[N]) {
        arg.typed.name = Name{name};
    }

    template<class Builder>
    auto type(Builder&& b) -> ArgumentBuilder {
        typeExprBuilder = [b2 = std::move(b)](const Scope& scope) mutable { return std::move(b2).build(scope); };
        return *this;
    }

    auto left() -> ArgumentBuilder {
        arg.side = ArgumentSide::left;
        return *this;
    }
    auto right() -> ArgumentBuilder {
        arg.side = ArgumentSide::right;
        return *this;
    }
    auto result() -> ArgumentBuilder {
        arg.side = ArgumentSide::result;
        return *this;
    }
    auto optional() -> ArgumentBuilder {
        arg.flags |= ArgumentFlag::optional;
        return *this;
    }

    auto build(const Scope& scope) && -> Argument {
        if (typeExprBuilder) {
            arg.typed.type = typeExprBuilder(scope);
        }
        return std::move(arg);
    }
};

} // namespace details

template<size_t N>
auto arg(const char (&name)[N]) {
    return details::ArgumentBuilder{name};
}

} // namespace instance
