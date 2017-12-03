#pragma once
#include "scope.h"

#include "function_builder.h"

namespace instance {

namespace details {

template<class Variant>
struct node_builder {
    static auto build(const scope_t &, Variant &&v) -> node_t { return std::move(v); }
};

template<>
struct node_builder<fun_builder> {
    static auto build(const scope_t &scope, fun_builder &&v) -> node_t { return node_t{std::move(v).build(scope)}; }
};

} // namespace details

template<class V>
inline auto build_node(const scope_t &scope, V &&v) -> decltype(auto) {
    return details::node_builder<V>::build(scope, std::forward<V>(v));
}

template<class... P>
inline auto build_scope(scope_t &scope, P &&... p) {
    scope.emplace(build_node(scope, std::forward<P>(p))...);
}

} // namespace instance
