#pragma once
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "scanner/StringLiteral.h"
#include "strings/Output.h"

#include "str.h"

// TODO: tmp
#include <iostream>

namespace intrinsic {

struct Context {};

// TODO: move
template<>
struct TypeOf<scanner::StringLiteral> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"StringLiteral"};
        info.size = sizeof(scanner::StringLiteral);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {}
};

template<>
struct TypeOf<Context> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"Context"};
        info.size = sizeof(Context);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    template<class Module>
    static constexpr auto module(Module&) {
        //        mod.template function<
        //            [] {
        //                auto info = FunctionInfo{};
        //                info.name = Name{".declareFunction"};
        //                info.flags = FunctionFlag::CompileTimeOnly;
        //                return info;
        //            },
        //            asPtr(&declareFunction)>(&declareFunction);

        //        mod.template function<
        //            [] {
        //                auto info = FunctionInfo{};
        //                info.name = Name{".declareVariable"};
        //                info.flags = FunctionFlag::CompileTimeOnly;
        //                return info;
        //            },
        //            asPtr(&declareVariable)>(&declareVariable);
    }
};

struct Rebuild {
    static constexpr auto info() {
        auto info = ModuleInfo{};
        info.name = Name{"Rebuild"};
        return info;
    }

    struct Literal {
        scanner::StringLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"literal"};
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    static void debugSay(const Literal& literal) {
        auto text = literal.v.text;
        std::cout << text << '\n';
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template function<
            [] {
                auto info = FunctionInfo{};
                info.name = Name{".say"};
                info.flags = FunctionFlag::CompileTimeOnly;
                return info;
            },
            asPtr(&debugSay)>(&debugSay);

        mod.template type<scanner::StringLiteral>();

        // mod.template type<compiler::Context>();
        // mod.template type<compiler::Scope>();
        // mod.template type<compiler::LocalScope>();

        //        mod.template function<
        //            [] {
        //                auto info = FunctionInfo{};
        //                info.name = Name{".context"};
        //                info.flags = FunctionFlag::CompileTimeOnly;
        //                return info;
        //            },
        //            asPtr(&currentContext)>(&currentContext);
    }
};

} // namespace intrinsic
