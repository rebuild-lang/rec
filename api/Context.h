#pragma once
#include "Basic.h"
#include "Instance.h"
#include "Literal.h"

#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "strings/Output.h"

// TODO: tmp
#include <iostream>

namespace intrinsic {

struct Context {};

template<>
struct TypeOf<Context> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{"Context"};
        info.size = sizeof(Context);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    struct Label {
        scanner::Token v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"label"};
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct Block {
        parser::block::BlockLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"block"};
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct ModuleResult {
        instance::Module* v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"result"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };

    static void declareModule(const Label& label, const Block& block, ModuleResult& res) {}

    template<class Module>
    static constexpr auto module(Module& mod) {
        //        mod.template function<&declareFunction,
        //            [] {
        //                auto info = FunctionInfo{};
        //                info.name = Name{".declareFunction"};
        //                info.flags = FunctionFlag::CompileTimeOnly;
        //                return info;
        //            }>();

        //        mod.template function<&declareVariable,
        //            [] {
        //                auto info = FunctionInfo{};
        //                info.name = Name{".declareVariable"};
        //                info.flags = FunctionFlag::CompileTimeOnly;
        //                return info;
        //            }>();

        mod.template function<&declareModule, [] {
            auto info = FunctionInfo{};
            info.name = Name{".declareModule"};
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }>();
    }
};

struct Rebuild {
    static constexpr auto info() {
        auto info = ModuleInfo{};
        info.name = Name{"Rebuild"};
        return info;
    }

    struct SayLiteral {
        scanner::StringLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"literal"};
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    static void debugSay(const SayLiteral& literal) {
        auto text = literal.v.text;
        std::cout << text << '\n';
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template module<Basic>();
        mod.template module<Literal>();
        mod.template module<Instance>();

        mod.template function<&debugSay, [] {
            auto info = FunctionInfo{};
            info.name = Name{".say"};
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }>();

        // mod.template type<compiler::Context>();
        // mod.template type<compiler::Scope>();
        // mod.template type<compiler::LocalScope>();

        //        mod.template function<&currentContext,
        //            [] {
        //                auto info = FunctionInfo{};
        //                info.name = Name{".context"};
        //                info.flags = FunctionFlag::CompileTimeOnly;
        //                return info;
        //            }>();
    }
};

} // namespace intrinsic
