#pragma once
#include "Basic.h"
#include "Instance.h"
#include "Literal.h"
#include "Parser.h"

#include "instance/IntrinsicContext.h"

#include "intrinsic/Function.h"
#include "intrinsic/Module.h"
#include "intrinsic/Type.h"

#include "strings/Rope.ostream.h"

// TODO(arBmind): remove dependency
#include <iostream>

namespace intrinsic {

template<>
struct TypeOf<Context*> {
    static constexpr auto info() {
        auto info = TypeInfo{};
        info.name = Name{".Context"};
        info.size = sizeof(Context);
        info.flags = TypeFlag::CompileTime;
        return info;
    }

    struct Label {
        parser::IdentifierLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"label"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct Block {
        parser::BlockLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"block"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
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
            // info.flags = ArgumentFlag::Assignable; // | ArgumentFlag::Optional;
            return info;
        }
    };

    static void declareModule(Label label, Block block, ModuleResult& res, ImplicitContext context) {
        (void)res;
        auto name = label.v.range.text;
        auto optNode = context.v->parserScope->locals[name];
        if (optNode) {
            auto node = optNode.value();
            if (node->holds<instance::Module>()) {
                auto& module = node->get<instance::Module>();
                auto moduleScope = instance::Scope(context.v->parserScope);
                moduleScope.locals = std::move(module.locals);
                context.v->parse(block.v, &moduleScope);
                module.locals = std::move(moduleScope.locals);
                res.v = &module;
            }
            else
                return; // error
        }
        else {
            auto optNode = context.v->parserScope->emplace([&] {
                auto module = instance::Module{};
                module.name = strings::to_string(name);
                auto moduleScope = instance::Scope(context.v->parserScope);
                context.v->parse(block.v, &moduleScope);
                module.locals = std::move(moduleScope.locals);
                return module;
            }());
            res.v = &optNode.value()->get<instance::Module>();
        }
    }

    struct Typed {
        parser::Typed v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct VariableInitResult {
        parser::VariableInit v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"result"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };

    static void declareVariable(Typed typed, VariableInitResult& res, ImplicitContext context) {
        new (&res.v) parser::VariableInit();

        if (!typed.v.name || !typed.v.type) {
            return; // error
        }
        auto name = typed.v.name.value();
        if (auto node = context.v->parserScope->locals[name]; node) {
            return; // error
        }
        auto optNode = context.v->parserScope->emplace([&] {
            auto variable = instance::Variable{};
            variable.typed.name = name;
            variable.typed.type = typed.v.type.value();
            return variable;
        }());
        if (!optNode) {
            // error
            return;
        }
        res.v.variable = &optNode.value()->get<instance::Variable>();
        if (typed.v.value) res.v.nodes.push_back(typed.v.value.value());
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
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

        mod.template function<&declareVariable, [] {
            auto info = FunctionInfo{};
            info.name = Name{".declareVariable"};
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
        parser::StringLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"literal"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    static void debugSay(SayLiteral literal) {
        auto text = literal.v.value.text;
        std::cout << text << '\n';
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template type<Context*>();

        mod.template module<Basic>();
        mod.template module<Literal>();
        mod.template module<Instance>();
        mod.template module<ParserModule>();

        mod.template function<&debugSay, [] {
            auto info = FunctionInfo{};
            info.name = Name{".say"};
            info.flags = FunctionFlag::CompileTimeOnly;
            return info;
        }>();

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
