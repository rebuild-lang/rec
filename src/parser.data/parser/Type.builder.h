#pragma once
#include "parser/Type.h"

#include "instance/ScopeLookup.h"

namespace parser {

namespace details {

struct TypeBuilder {
    Name name{};

    TypeBuilder() = default;

    template<size_t N>
    TypeBuilder(const char (&name)[N])
        : name(Name{name}) {}

    operator bool() const { return !name.isEmpty(); }

    auto build(const instance::Scope& scope) && -> TypeView {
        const auto& t = instance::lookupA<instance::TypePtr>(scope, name);
        return t.get();
    }
};

} // namespace details

template<size_t N>
inline auto type(const char (&name)[N]) -> details::TypeBuilder {
    return {name};
}

} // namespace parser
