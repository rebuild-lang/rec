#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "strings/String.ostream.h"
#include "strings/View.ostream.h"

#include <cassert>
#include <iostream>

namespace intrinsic {

// example visitor implemenetation
struct ModuleOutput {
    using This = ModuleOutput;

    template<class T>
    void type() {
        constexpr auto info = TypeOf<T>::info();
        std::cout << indent << "type " << info.name << '\n';
        if constexpr (info.flags.any(TypeFlag::Instance)) {
            // TODO(arBmind)
        }
        else {
            indent += "  ";
            TypeOf<T>::module(*this);
            indent.resize(indent.size() - 2);
        }
    }

    template<class T>
    void module() {
        auto info = T::info();
        std::cout << indent << "module " << info.name << '\n';
        indent += "  ";
        T::module(*this);
        indent.resize(indent.size() - 2);
    }

    template<auto* F, FunctionInfoFunc Info>
    void function() {
        return functionImpl<Info, F>(makeSignature(F));
    }

private:
    std::string indent{};

    template<FunctionInfoFunc Info, auto* F, class... Params>
    void functionImpl(FunctionSignature<void, Params...>) {
        auto info = Info();
        std::cout << indent << "function " << info.name << '\n';
        indent += "  ";
        (parameter<Params>(), ...);
        indent.resize(indent.size() - 2);
    }

    template<class T>
    void parameter() {
        std::cout << indent << "param " << Parameter<T>::info().name //
                  << " : " << Parameter<T>::typeInfo().name << '\n';
    }
};

} // namespace intrinsic
