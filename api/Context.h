#pragma once
#include "Basic.h"
#include "Instance.h"
#include "Literal.h"

#include "intrinsic/Context.h"
#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "strings/Output.h"

// TODO: tmp
#include <iostream>

namespace intrinsic {

template<>
struct TypeOf<Context> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Context"};
        info.size = sizeof(Context);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    struct Label {
        parser::expression::IdentifierLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"label"};
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct Block {
        parser::expression::BlockLiteral v;
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
    struct ImplicitContext {
        Context* v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"context"};
            info.side = ArgumentSide::Implicit;
            info.flags = ArgumentFlag::Assignable; // | ArgumentFlag::Optional;
            return info;
        }
    };

    static void declareModule(const Label& label, const Block& block, ModuleResult res, ImplicitContext context) {
        (void)label;
        (void)block;
        (void)res;
        (void)context;
        auto name = label.v.range.text;
        auto node = context.v->parserScope->locals[name];
        if (node) {
            if (node->holds<instance::Module>()) {
                auto& module = node->get<instance::Module>();
                auto moduleScope = instance::Scope(context.v->parserScope);
                moduleScope.locals = std::move(module.locals);
                context.v->parse(block.v, &moduleScope);
                module.locals = std::move(moduleScope.locals);
            }
            else
                return; // error
        }
        else {
            auto module = instance::Module{};
            module.name = name;
            auto moduleScope = instance::Scope(context.v->parserScope);
            context.v->parse(block.v, &moduleScope);
            module.locals = std::move(moduleScope.locals);
            context.v->parserScope->emplace(std::move(module));
        }
        // auto module = context.v->fetchOrCreateModule(label.v);
        // context.v->parseModuleBlock(module, block);
        // res.v = module;
    }

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
        parser::expression::StringLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"literal"};
            info.side = ArgumentSide::Right;
            info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    static void debugSay(const SayLiteral& literal) {
        auto text = literal.v.value.text;
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

        mod.template type<Context>();
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
