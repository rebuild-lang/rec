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
struct TypeOf<ContextInterface*> {
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
        parser::ScopedBlockLiteral v;
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
        ContextInterface* v;
        static constexpr auto info() {
            return ParameterInfo{Name{"__context__"}, ParameterSide::Implicit};
            // info.flags = ParameterFlag::Assignable; // | ParameterFlag::Optional;
        }
    };

    static void declareModule(Label label, Block block, ModuleResult& res, ImplicitContext context) {
        auto name = label.v.input;
        auto range = context.v->parserScope->locals->byName(name);
        if (!range.empty()) {
            auto& node = range.frontValue();
            if (range.single() && node.holds<instance::ModulePtr>()) {
                auto& module = node.get<instance::ModulePtr>();
                auto localsPtr = instance::LocalScopePtr(module, &module->locals);
                auto moduleScope = std::make_shared<instance::Scope>(std::move(localsPtr), context.v->parserScope);
                auto parsedBlock = context.v->parse(block.v.block, moduleScope);
                (void)parsedBlock; // TODO(arBmind): use parsedBlock
                res.v = module.get();
            }
            else {
                using namespace diagnostic;
                auto doc =
                    Document{{Paragraph{String("The name of the module is already taken and it's not a module"), {}}}};
                // TODO: somehow extract the source code line.

                auto expl = Explanation{String("Module Name already defined"), doc};
                context.v->report(Diagnostic{Code{String{"rebuild-api"}, 1}, Parts{expl}});

                res.v = nullptr;
                return; // error
            }
        }
        else {
            auto module = [&] {
                auto module = std::make_shared<instance::Module>();
                module->name = strings::to_string(name);
                context.v->parserScope->emplace(module);

                auto localsPtr = instance::LocalScopePtr(module, &module->locals);
                auto moduleScope = std::make_shared<instance::Scope>(std::move(localsPtr), context.v->parserScope);
                auto parsedBlock = context.v->parse(block.v.block, moduleScope);
                (void)parsedBlock; // TODO(arBmind): use parsedBlock
                return module;
            }();

            res.v = module.get();
        }
    }

    struct NameTypeValue {
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

    static auto typeFromNode(const parser::TypeExpr& node) -> parser::TypeView {
        // TODO(arBmind): somehow handle computed types
        return node.visit(
            [](const parser::TypeReference& tr) { return tr.type; }, //
            [](const parser::ModuleReference& mr) -> parser::TypeView {
                auto typeRange = mr.module->locals.byName(parser::nameOfType());
                if (typeRange.single()) {
                    const auto& type = typeRange.frontValue().get<instance::TypePtr>();
                    return type.get();
                }
                return {}; // error module is not a type
            },
            [](const auto&) -> parser::TypeView { return {}; } // not a type
        );
    }

    static void declareVariable(NameTypeValue ntv, VariableInitResult& res, ImplicitContext context) {
        if (!ntv.v.name || !ntv.v.type) {
            return; // error
        }
        auto name = ntv.v.name.value();
        if (auto range = context.v->parserScope->locals->byName(name); !range.empty()) {
            return; // error
        }
        auto variable = [&] {
            auto variable = std::make_shared<instance::Variable>();
            variable->name = name;
            if (ntv.v.type) variable->type = typeFromNode(ntv.v.type.value());
            // TODO(arBmind): else use type of value!
            return variable;
        }();

        context.v->parserScope->emplace(variable);
        res.v.variable = variable.get();

        if (ntv.v.value) res.v.nodes.push_back(ntv.v.value.value());
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
        auto range = context.v->parserScope->locals->byName(name);
        if (!range.empty() && !range.frontValue().holds<instance::FunctionPtr>()) {
            /*
                return; // error
            */
        }
        else {
            auto function = [&] {
                auto function = std::make_shared<instance::Function>();
                function->name = strings::to_string(name);
                function->flags |= instance::FunctionFlag::compile_time; // TODO(arBmind): allow custom flags

                auto addParametersFromNtvTuple = [&](instance::ParameterSide side,
                                                     parser::NameTypeValueTuple& ntvTuple) {
                    for (auto& ntv : ntvTuple.tuple) {
                        // TODO(arBmind): check double parameter names
                        auto parameter = [&] {
                            auto parameter = std::make_shared<instance::Parameter>();
                            if (ntv.name) parameter->name = strings::to_string(ntv.name.value());
                            if (ntv.type) parameter->type = ntv.type.value();

                            // TODO(arBmind): else type of value / error
                            parameter->side = side;
                            if (side == instance::ParameterSide::result)
                                parameter->flags |= instance::ParameterFlag::assignable;

                            if (ntv.value) parameter->defaultValue.push_back(ntv.value.value());
                            return parameter;
                        }();
                        auto variable = [&] {
                            auto variable = std::make_shared<instance::Variable>();
                            if (ntv.name) variable->name = strings::to_string(ntv.name.value());
                            variable->flags = instance::VariableFlag::function_parameter;
                            if (parameter->flags.any(instance::ParameterFlag::assignable))
                                variable->flags |= instance::VariableFlag::assignable;

                            variable->parameter = parameter.get();

                            return variable;
                        }();
                        parameter->variable = variable.get();

                        function->parameters.emplace_back(parameter);
                        function->parameterScope.emplace(variable);
                    }
                };

                addParametersFromNtvTuple(instance::ParameterSide::left, left.v);
                addParametersFromNtvTuple(instance::ParameterSide::right, right.v);
                addParametersFromNtvTuple(instance::ParameterSide::result, results.v);
                return function;
            }();

            context.v->parserScope->emplace(function);
            res.v = function.get();

            // parse function body
            auto parameterLocalScope = instance::LocalScopePtr(function, &function->parameterScope);
            auto parameterScope = std::make_shared<instance::Scope>(parameterLocalScope, context.v->parserScope);

            auto& localBlock = function->body.get<instance::ParsedBlock>();
            auto blockLocalScope = instance::LocalScopePtr(function, &localBlock.locals);
            auto bodyScope = std::make_shared<instance::Scope>(blockLocalScope, parameterScope);

            localBlock.block = context.v->parse(block.v.block, bodyScope);
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
        mod.template type<ContextInterface*>();

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
