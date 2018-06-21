#pragma once
#include "Argument.h"

#include "Scope.h"
#include "ScopeLookup.h"

namespace instance {

namespace details {

struct ArgumentBuilder {
    using This = ArgumentBuilder;
    Argument arg;
    Name typeName;

    template<size_t N>
    ArgumentBuilder(const char (&name)[N]) {
        arg.typed.name = Name{name};
    }

    template<size_t N>
    auto type(const char (&type)[N]) -> ArgumentBuilder {
        typeName = Name{type};
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
        if (!typeName.isEmpty()) {
            auto* t = &lookupA<Type>(scope, typeName);
            arg.typed.type = parser::expression::TypeInstance{t};
        }
        return std::move(arg);
    }
};

} // namespace details

template<size_t N>
inline auto arg(const char (&name)[N]) -> details::ArgumentBuilder {
    return {name};
}

} // namespace instance
