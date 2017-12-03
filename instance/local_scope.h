#pragma once
#include "strings/utf8_view.h"

#include <map>

namespace instance {

using view_t = strings::utf8_view;
class node_t;

using name_map = std::map<view_t, node_t>;

struct local_scope_t {
    using this_t = local_scope_t;
    name_map names;

    local_scope_t() = default;

    // non copyable
    local_scope_t(const this_t &) = delete;
    auto operator=(const this_t &) -> this_t & = delete;
    // move enabled
    local_scope_t(this_t &&) = default;
    auto operator=(this_t &&) -> this_t & = default;

    auto operator[](const view_t &name) const & -> const node_t *;

    template<class T, class... Ts>
    bool emplace(T &&arg, Ts &&... args) & {
        bool all_success = emplace_impl(std::forward<T>(arg));
        if constexpr (0 != sizeof...(Ts)) {
            auto x = {(all_success &= emplace_impl(std::forward<Ts>(args)))...};
            (void)x;
        }
        return all_success;
    }

private:
    bool emplace_impl(node_t &&instance) &;

    // bool replace(old, new)
};

} // namespace instance
