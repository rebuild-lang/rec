#pragma once
#include "function.h"
#include "type.h"
#include "variable.h"

#include "meta/variant.h"
#include "strings/utf8_view.h"

#include <map>

namespace instance {

using variant_t = meta::variant<function_t, variable_t, argument_t, type_t>;
using view_t = strings::utf8_view;

inline auto name_of(const variant_t &instance) -> view_t {
    return instance.visit(
        [](const function_t &f) -> view_t { return f.name; },
        [](const variable_t &v) -> view_t { return v.name; },
        [](const argument_t &a) -> view_t { return a.name; },
        [](const type_t &t) -> view_t { return t.name; });
}

using name_map = std::map<view_t, variant_t>;

struct scope_t {
    using this_t = scope_t;
    const scope_t *parent{};
    name_map locals;

    scope_t() = default;
    explicit scope_t(const this_t *parent)
        : parent(parent) {}

    // non copyable
    scope_t(const this_t &) = delete;
    this_t &operator=(const this_t &) = delete;
    scope_t(this_t &&) = default;
    this_t &operator=(this_t &&) = default;

    auto operator[](const view_t &name) const -> const variant_t * {
        auto it = locals.find(name);
        if (it == locals.end()) {
            if (parent) return (*parent)[name];
            return nullptr;
        }
        return &it->second;
    }

    template<class T, class... Ts>
    bool emplace(T &&arg, Ts &&... args) {
        bool done = emplace_impl(std::forward<T>(arg));
        if constexpr (0 != sizeof...(Ts)) {
            auto x = {(done &= emplace_impl(std::forward<Ts>(args)))...};
            (void)x;
        }
        return done;
    }

private:
    bool emplace_impl(variant_t &&instance) {
        const auto &name = name_of(instance);
        if (locals.end() != locals.find(name)) return false;
        locals.insert({name, std::move(instance)});
        return true;
    }

    // bool replace(old, new)
};

} // namespace instance
