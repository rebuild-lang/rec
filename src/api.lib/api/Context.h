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
            info.name = Name{"__label__"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct Block {
        parser::BlockLiteral v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__block__"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct ModuleResult {
        instance::Module* v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__result__"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };
    struct ImplicitContext {
        Context* v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__context__"};
            info.side = ArgumentSide::Implicit;
            // info.flags = ArgumentFlag::Assignable; // | ArgumentFlag::Optional;
            return info;
        }
    };

    static void declareModule(Label label, Block block, ModuleResult& res, ImplicitContext context) {
        auto name = label.v.input;
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
            else {
                using namespace diagnostic;
                auto doc =
                    Document{{Paragraph{String("The name of the module is already taken and it's not a module"), {}}}};
                // TODO: somehow extract the source code line.

                auto expl = Explanation{String("Module Name already defined"), doc};
                context.v->report(Diagnostic{Code{String{"rebuild-api"}, 1}, Parts{expl}});

                res.v = new instance::Module{};
                return; // error
            }
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
        parser::NameTypeValue v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__typed__"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct VariableInitResult {
        parser::VariableInit v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__variable_init__"};
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

    struct LeftArgumentTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__left__"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct RightArgumentTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__right__"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct ResultArgumentTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__result__"};
            info.side = ArgumentSide::Right;
            // info.flags = ArgumentFlag::Reference;
            return info;
        }
    };
    struct FunctionResult {
        instance::Function* v;
        static constexpr auto info() {
            auto info = ArgumentInfo{};
            info.name = Name{"__function__"};
            info.side = ArgumentSide::Result;
            info.flags = ArgumentFlag::Assignable;
            return info;
        }
    };

    static void declareFunction(
        LeftArgumentTuple left,
        Label label,
        RightArgumentTuple right,
        ResultArgumentTuple results,
        Block block,
        FunctionResult& res,
        ImplicitContext context) {

        // TODO(arBmind): implement
        auto name = label.v.input;
        auto optNode = context.v->parserScope->locals[name];
        if (optNode) {
            /*
            auto node = optNode.value();
            if (node->holds<instance::Function>()) {
                auto& module = node->get<instance::Function>();
                auto moduleScope = instance::Scope(context.v->parserScope);
                moduleScope.locals = std::move(module.locals);
                context.v->parse(block.v, &moduleScope);
                module.locals = std::move(moduleScope.locals);
                res.v = &module;
            }
            else
                return; // error
            */
        }
        else {
            auto argumentScope = instance::Scope(context.v->parserScope);
            auto optNode = context.v->parserScope->emplace([&] {
                auto function = instance::Function{};
                function.name = strings::to_string(name);
                function.flags |= instance::FunctionFlag::compiletime; // TODO(arBmind): allow custom flags

                auto addArgumentsFromTyped = [&](instance::ArgumentSide side, parser::NameTypeValueTuple& tuple) {
                    for (auto& typed : tuple.tuple) {
                        auto optView = argumentScope.emplace([&] {
                            auto argument = instance::Argument{};
                            if (typed.name) argument.typed.name = strings::to_string(typed.name.value());
                            if (typed.type) argument.typed.type = typed.type.value();
                            argument.side = side;
                            if (side == instance::ArgumentSide::result)
                                argument.flags |= instance::ArgumentFlag::assignable;

                            if (typed.value) argument.init.push_back(typed.value.value());
                            return argument;
                        }());
                        if (optView) function.arguments.push_back(&optView.value()->get<instance::Argument>());
                    }
                };

                addArgumentsFromTyped(instance::ArgumentSide::left, left.v);
                addArgumentsFromTyped(instance::ArgumentSide::right, right.v);
                addArgumentsFromTyped(instance::ArgumentSide::result, results.v);
                return function;
            }());
            auto& function = optNode.value()->get<instance::Function>();

            auto bodyScope = instance::Scope(&argumentScope);
            function.body.block = context.v->parse(block.v, &bodyScope);
            function.body.locals = std::move(bodyScope.locals);

            function.argumentScope = std::move(argumentScope.locals);

            res.v = &function;
        }
    }

    template<class Module>
    static constexpr auto module(Module& mod) {
        mod.template function<&declareModule, [] {
            auto info = FunctionInfo{};
            info.name = Name{".declareModule"};
            info.flags = FunctionFlag::CompileTimeSideEffects;
            return info;
        }>();

        mod.template function<&declareVariable, [] {
            auto info = FunctionInfo{};
            info.name = Name{".declareVariable"};
            info.flags = FunctionFlag::CompileTimeSideEffects;
            return info;
        }>();

        mod.template function<&declareFunction, [] {
            auto info = FunctionInfo{};
            info.name = Name{".declareFunction"};
            info.flags = FunctionFlag::CompileTimeSideEffects;
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
            info.flags = FunctionFlag::CompileTimeSideEffects;
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
