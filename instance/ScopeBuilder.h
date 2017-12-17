#pragma once
#include "Scope.h"

#include "FunctionBuilder.h"

namespace instance {

namespace details {

template<class Variant>
struct NodeBuilder {
    static auto build(const Scope &, Variant &&v) -> Node { return std::move(v); }
};

template<>
struct NodeBuilder<FunctionBuilder> {
    static auto build(const Scope &scope, FunctionBuilder &&v) -> Node { return Node{std::move(v).build(scope)}; }
};

} // namespace details

template<class V>
inline auto buildNode(const Scope &scope, V &&v) -> decltype(auto) {
    return details::NodeBuilder<V>::build(scope, std::forward<V>(v));
}

template<class... P>
inline auto buildScope(Scope &scope, P &&... p) {
    scope.emplace(buildNode(scope, std::forward<P>(p))...);
}

} // namespace instance
