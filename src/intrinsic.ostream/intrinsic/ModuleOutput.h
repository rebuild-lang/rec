#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "meta/Pointer.h"
#include "strings/String.ostream.h"
#include "strings/View.ostream.h"

#include <cassert>
#include <iostream>

namespace intrinsic {

using meta::Ptr;
using meta::ptr_to;

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

    template<auto* F>
    void function(Ptr<F>*, const FunctionInfo& info) {
        return functionImpl(info, makeSignature(F));
    }

private:
    std::string indent{};

    template<class... Params>
    void functionImpl(const FunctionInfo& info, FunctionSignature<void, Params...>) {
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
