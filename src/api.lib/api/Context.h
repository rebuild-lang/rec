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
            auto info = ParameterInfo{};
            info.name = Name{"__label__"};
            info.side = ParameterSide::Right;
            // info.flags = ParameterFlag::Reference;
            return info;
        }
    };
    struct Block {
        parser::BlockLiteral v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__block__"};
            info.side = ParameterSide::Right;
            // info.flags = ParameterFlag::Reference;
            return info;
        }
    };
    struct ModuleResult {
        instance::Module* v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__result__"};
            info.side = ParameterSide::Result;
            info.flags = ParameterFlag::Assignable;
            return info;
        }
    };
    struct ImplicitContext {
        Context* v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__context__"};
            info.side = ParameterSide::Implicit;
            // info.flags = ParameterFlag::Assignable; // | ParameterFlag::Optional;
            return info;
        }
    };

    static void declareModule(Label label, Block block, ModuleResult& res, ImplicitContext context) {
        auto name = label.v.input;
        auto range = context.v->parserScope->locals[name];
        if (!range.empty()) {
            auto& node = range.frontValue();
            if (range.single() && node.holds<instance::Module>()) {
                auto& module = node.get<instance::Module>();
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
            auto node = context.v->parserScope->emplace([&] {
                auto module = instance::Module{};
                module.name = strings::to_string(name);
                auto moduleScope = instance::Scope(context.v->parserScope);
                context.v->parse(block.v, &moduleScope);
                module.locals = std::move(moduleScope.locals);
                return module;
            }());
            res.v = &node->get<instance::Module>();
        }
    }

    struct Typed {
        parser::NameTypeValue v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__typed__"};
            info.side = ParameterSide::Right;
            // info.flags = ParameterFlag::Reference;
            return info;
        }
    };
    struct VariableInitResult {
        parser::VariableInit v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__variable_init__"};
            info.side = ParameterSide::Result;
            info.flags = ParameterFlag::Assignable;
            return info;
        }
    };

    static void declareVariable(Typed typed, VariableInitResult& res, ImplicitContext context) {
        new (&res.v) parser::VariableInit();

        if (!typed.v.name || !typed.v.type) {
            return; // error
        }
        auto name = typed.v.name.value();
        if (auto range = context.v->parserScope->locals[name]; !range.empty()) {
            return; // error
        }
        auto node = context.v->parserScope->emplace([&] {
            auto variable = instance::Variable{};
            variable.typed.name = name;
            variable.typed.type = typed.v.type.value();
            return variable;
        }());
        res.v.variable = &node->get<instance::Variable>();
        if (typed.v.value) res.v.nodes.push_back(typed.v.value.value());
    }

    struct LeftParameterTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__left__"};
            info.side = ParameterSide::Right;
            // info.flags = ParameterFlag::Reference;
            return info;
        }
    };
    struct RightParameterTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__right__"};
            info.side = ParameterSide::Right;
            // info.flags = ParameterFlag::Reference;
            return info;
        }
    };
    struct ResultParameterTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__result__"};
            info.side = ParameterSide::Right;
            // info.flags = ParameterFlag::Reference;
            return info;
        }
    };
    struct FunctionResult {
        instance::Function* v;
        static constexpr auto info() {
            auto info = ParameterInfo{};
            info.name = Name{"__function__"};
            info.side = ParameterSide::Result;
            info.flags = ParameterFlag::Assignable;
            return info;
        }
    };

    static void declareFunction(
        LeftParameterTuple left,
        Label label,
        RightParameterTuple right,
        ResultParameterTuple results,
        Block block,
        FunctionResult& res,
        ImplicitContext context) {

        // TODO(arBmind): implement
        auto name = label.v.input;
        auto range = context.v->parserScope->locals[name];
        if (!range.empty() && !range.frontValue().holds<instance::Function>()) {
            /*
                return; // error
            */
        }
        else {
            auto parameterScope = instance::Scope(context.v->parserScope);
            auto node = context.v->parserScope->emplace([&] {
                auto function = instance::Function{};
                function.name = strings::to_string(name);
                function.flags |= instance::FunctionFlag::compiletime; // TODO(arBmind): allow custom flags

                auto addParametersFromTyped = [&](instance::ParameterSide side, parser::NameTypeValueTuple& tuple) {
                    for (auto& typed : tuple.tuple) {
                        // TODO(arBmind): check double parameter names
                        auto view = parameterScope.emplace([&] {
                            auto parameter = instance::Parameter{};
                            if (typed.name) parameter.typed.name = strings::to_string(typed.name.value());
                            if (typed.type) parameter.typed.type = typed.type.value();
                            parameter.side = side;
                            if (side == instance::ParameterSide::result)
                                parameter.flags |= instance::ParameterFlag::assignable;

                            if (typed.value) parameter.init.push_back(typed.value.value());
                            return parameter;
                        }());
                        function.parameters.push_back(&view->get<instance::Parameter>());
                    }
                };

                addParametersFromTyped(instance::ParameterSide::left, left.v);
                addParametersFromTyped(instance::ParameterSide::right, right.v);
                addParametersFromTyped(instance::ParameterSide::result, results.v);
                return function;
            }());
            auto& function = node->get<instance::Function>();

            auto bodyScope = instance::Scope(&parameterScope);
            function.body.block = context.v->parse(block.v, &bodyScope);
            function.body.locals = std::move(bodyScope.locals);

            function.parameterScope = std::move(parameterScope.locals);

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
            auto info = ParameterInfo{};
            info.name = Name{"literal"};
            info.side = ParameterSide::Right;
            // info.flags = ParameterFlag::Reference;
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
