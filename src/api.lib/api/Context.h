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
        return TypeInfo{Name{".Context"}, TypeFlag::CompileTime}; //
    }

    struct Label {
        parser::IdentifierLiteral v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__label__"}, ParameterSide::Right}; //
        }
    };
    struct Block {
        parser::BlockLiteral v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__block__"}, ParameterSide::Right}; //
        }
    };
    struct ModuleResult {
        instance::Module* v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__result__"}, ParameterSide::Result, ParameterFlag::Assignable};
        }
    };
    struct ImplicitContext {
        Context* v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__context__"}, ParameterSide::Implicit};
            // info.flags = ParameterFlag::Assignable; // | ParameterFlag::Optional;
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
            return ParameterInfo{Name{"__typed__"}, ParameterSide::Right}; //
        }
    };
    struct VariableInitResult {
        parser::VariableInit v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__variable_init__"}, ParameterSide::Result, ParameterFlag::Assignable};
        }
    };

    static auto typeFromNode(const parser::Node& node) -> parser::TypeView {
        // TODO(arBmind): somehow handle computed types
        return node.visit(
            [](const parser::TypeReference& tr) { return tr.type; }, //
            [](const parser::ModuleReference& mr) -> parser::TypeView {
                auto typeRange = mr.module->locals[instance::nameOfType()];
                if (typeRange.single()) {
                    const auto& type = typeRange.frontValue().get<instance::Type>();
                    return &type;
                }
                return {}; // error module is not a type
            },
            [](const auto&) -> parser::TypeView { return {}; } // not a type
        );
    }

    static void declareVariable(Typed typed, VariableInitResult& res, ImplicitContext context) {
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
            if (typed.v.type) variable.typed.type = typeFromNode(typed.v.type.value());
            // TODO(arBmind): else use type of value!
            return variable;
        }());
        res.v.variable = &node->get<instance::Variable>();
        if (typed.v.value) res.v.nodes.push_back(typed.v.value.value());
    }

    struct LeftParameterTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            return ParameterInfo{Name{"left"}, ParameterSide::Right}; //
        }
    };
    struct RightParameterTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__right__"}, ParameterSide::Right}; //
        }
    };
    struct ResultParameterTuple {
        parser::NameTypeValueTuple v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__result__"}, ParameterSide::Right}; //
        }
    };
    struct FunctionResult {
        instance::Function* v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__function__"}, ParameterSide::Result, ParameterFlag::Assignable};
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
                            if (typed.type) parameter.typed.type = typeFromNode(typed.type.value());
                            // TODO(arBmind): else type of value / error
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
            return FunctionInfo{Name{".declareModule"}, FunctionFlag::CompileTimeSideEffects};
        }>();

        mod.template function<&declareVariable, [] {
            return FunctionInfo{Name{".declareVariable"}, FunctionFlag::CompileTimeSideEffects};
        }>();

        mod.template function<&declareFunction, [] {
            return FunctionInfo{Name{".declareFunction"}, FunctionFlag::CompileTimeSideEffects};
        }>();
    }
};

struct Rebuild {
    static constexpr auto info() {
        return ModuleInfo{Name{"Rebuild"}}; //
    }

    struct SayLiteral {
        parser::StringLiteral v;
        static constexpr auto info() {
            return ParameterInfo{Name{"literal"}, ParameterSide::Right}; //
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
            return FunctionInfo{Name{".say"}, FunctionFlag::CompileTimeSideEffects};
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
