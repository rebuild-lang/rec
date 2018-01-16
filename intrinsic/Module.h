#pragma once
#include "Function.h"
#include "Type.h"

// debug only
#include <iostream>

namespace intrinsic {

struct ModuleInfo {
    const char* name{};
};

/* a module should look like this:
 *
 * class MyModule {
 *   constexpr static auto info() -> ModuleInfo;
 *
 *   template<class Module>
 *   constexpr static auto module(Module& mod) {
 *      mod.template type<Type>();
 *      mod.template function<&func, &funcInfo>();
 *      mod.template module<OtherModule>();
 *   }
 * };
 *
 * use visitor pattern to capture desired details
 */

// example visitor implemenetation
struct PrintVisitor {
    using This = PrintVisitor;

    template<class T>
    void type() {
        constexpr auto info = TypeOf<T>::info();
        std::cout << indent << "type " << info.name << '\n';
        if constexpr (info.flags.any(TypeFlag::Instance)) {
            // TODO
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

    using FunctionInfoFunc = FunctionInfo (*)();

    template<FunctionInfoFunc Info, class... Args>
    void function(void (*func)(Args...)) {
        auto info = Info();
        std::cout << indent << "function " << info.name << '\n';
        indent += "  ";
        (argument<Args>(), ...);
        indent.resize(indent.size() - 2);
    }

private:
    std::string indent{};

    template<class T>
    void argument() {
        std::cout << indent << "arg " << Argument<T>::info().name //
                  << " : " << Argument<T>::typeInfo().name << '\n';
    }
};

} // namespace intrinsic
