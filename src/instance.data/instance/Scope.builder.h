#pragma once
#include "Scope.h"

#include "Function.builder.h"

namespace instance {

namespace details {

template<class Variant>
struct EntryBuilder {
    static auto build(const Scope&, Variant&& v) -> Entry { return Entry{std::move(v).build()}; }
};

template<>
struct EntryBuilder<FunctionBuilder> {
    static auto build(const Scope& scope, FunctionBuilder&& builder) -> Entry {
        return Entry{std::move(builder).build(scope)};
    }
};

} // namespace details

template<class V>
auto buildEntry(const Scope& scope, V&& v) -> decltype(auto) {
    return details::EntryBuilder<V>::build(scope, std::forward<V>(v));
}

template<class... P>
auto buildScope(Scope& scope, P&&... p) {
    (scope.emplace(buildEntry(scope, std::forward<P>(p))), ...);
}

} // namespace instance
