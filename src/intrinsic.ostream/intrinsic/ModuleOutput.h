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

    template<FunctionInfoFunc Info, auto* F, class... Args>
    void functionImpl(FunctionSignature<void, Args...>) {
        auto info = Info();
        std::cout << indent << "function " << info.name << '\n';
        indent += "  ";
        (argument<Args>(), ...);
        indent.resize(indent.size() - 2);
    }

    template<class T>
    void argument() {
        std::cout << indent << "arg " << Argument<T>::info().name //
                  << " : " << Argument<T>::typeInfo().name << '\n';
    }
};

} // namespace intrinsic
