#pragma once
#include "instance/Scope.h"
#include "instance/ScopeLookup.h"
#include "instance/Type.h"
#include "intrinsic/Function.h"

namespace intrinsic {

template<class Type>
struct ResolveType {
    using This = ResolveType;

    const instance::LocalScope* scope;
    instance::TypeView result{};

    template<class T>
    static auto moduleInstance(const instance::Scope* scope) -> instance::TypeView {
        auto r = This{&scope->locals};
        r.template module<T>();
        return r.result;
    }

    template<class T>
    void type() {
        if constexpr (std::is_same_v<T, Type>) {
            constexpr auto info = TypeOf<T>::info();
            auto& mod = (*scope)[info.name].frontValue().template get<instance::Module>();
            result = &mod.locals[strings::View{"type"}].frontValue().template get<instance::Type>();
        }
    }

    template<class T>
    void module() {
        auto info = T::info();
        auto modRange = (*scope)[info.name];
        auto inner = This{&modRange.frontValue().template get<instance::Module>().locals};
        T::module(inner);
        if (inner.result) result = inner.result;
    }

    template<auto* F, FunctionInfoFunc Info>
    void function() {}
};

} // namespace intrinsic
