#pragma once
#include "local_scope.h"
#include "node.h"

namespace instance {

struct scope_t {
    using this_t = scope_t;
    const scope_t *parent{};
    local_scope_t locals;

    scope_t() = default;
    explicit scope_t(const this_t *parent)
        : parent(parent) {}

    // move enabled
    scope_t(this_t &&) = default;
    auto operator=(this_t &&) -> this_t & = default;

    auto operator[](const view_t &name) const & -> const node_t * {
        auto node = locals[name];
        if (!node && parent) return (*parent)[name];
        return node;
    }

    template<class T, class... Ts>
    bool emplace(T &&arg, Ts &&... args) & {
        return locals.emplace(std::forward<T>(arg), std::forward<Ts>(args)...);
    }
};

} // namespace instance
