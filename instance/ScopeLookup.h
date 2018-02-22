#pragma once
#include "Module.h"
#include "Scope.h"

namespace instance {

inline auto lookup(const Scope& scope, const Name& name) -> const Node& {
    auto it = name.begin();
    auto end = name.end();
    auto it2 = std::find(it, end, '.');
    auto c = scope[Name(it, it2)];
    if (!c) throw "name not found";
    while (it2 != end) {
        it = it2;
        it2 = std::find(it, end, '.');
        auto cp = c;
        cp->visit(
            [&](const Module& m) -> decltype(auto) {
                c = m.locals[Name{it, it2}];
            },
            [](const auto&) { throw "not a module!"; } //
        );
        if (!c) throw "nested name not found";
    }
    return *c;
}

template<class T>
auto lookupA(const Scope& scope, const Name& name) -> const T& {
    const auto& c = lookup(scope, name);
    if (!c.holds<T>()) {
        if constexpr (std::is_same_v<T, Type>) {
            const auto& m = c.get<Module>();
            auto* t = m.locals[Name{"type"}];
            if (!t) throw "wrong type";
            return t->get<T>();
        }
        throw "wrong type";
    }
    return c.get<T>();
}

} // namespace instance
