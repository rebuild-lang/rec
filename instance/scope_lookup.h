#pragma once
#include "module.h"
#include "scope.h"

namespace instance {

inline auto lookup(const scope_t &scope, const view_t &name) -> const node_t & {
    auto it = name.begin();
    auto end = name.end();
    auto it2 = std::find(it, end, '.');
    auto c = scope[view_t(it, it2)];
    if (!c) throw "name not found";
    while (it2 != end) {
        it = it2;
        it2 = std::find(it, end, '.');
        auto cp = c;
        cp->visit(
            [&](const module_t &m) -> decltype(auto) {
                c = m.locals[view_t{it, it2}];
            },
            [](const auto &) { throw "not a module!"; } //
        );
        if (!c) throw "nested name not found";
    }
    return *c;
}

template<class T>
auto lookup_a(const scope_t &scope, const view_t &name) -> const T & {
    const auto &c = lookup(scope, name);
    if (!c.holds<T>()) throw "wrong type";
    return c.get<T>();
}

} // namespace instance
