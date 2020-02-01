#pragma once
#include "execution/Frame.h"
#include "execution/Stack.h"

#include "parser/Expression.h"

#include "instance/Function.h"
#include "instance/IntrinsicContext.h"
#include "instance/Scope.h"
#include "instance/Variable.h"

#include <cassert>
#include <functional>

namespace execution {

// TODO(arBmind): somehow fix result / assignable (note: localFrame[TypeView] uses wrong type!
// TODO(arBmind): run destructors on frames!
// TODO(arBmind): assign defaults to results if unused!

using ParseBlock = std::function<parser::Block(const nesting::BlockLiteral& block, instance::Scope* scope)>;
using ReportDiagnositc = std::function<void(diagnostic::Diagnostic)>;

struct Compiler {
    Stack stack{}; // stack allocator
    ParseBlock parseBlock{};
    ReportDiagnositc reportDiagnostic = [](diagnostic::Diagnostic) {};
};

struct Context {
    const Context* parent{};
    Compiler* compiler{};

    const Context* caller{};
    instance::Scope* parserScope{};

    Byte* localBase{};
    LocalFrame localFrame{};

    auto operator[](instance::TypedView typed) const& -> Byte* {
        auto addr = localFrame[typed];
        if (addr) return addr;
        if (parent) return (*parent)[typed];
        return nullptr;
    }

    auto createCall() const& -> Context {
        auto result = Context{};
        result.compiler = compiler;
        result.caller = this;
        result.parserScope = parserScope;
        return result;
    }
    auto createNested() const& -> Context {
        auto result = Context{};
        result.parent = this;
        result.compiler = compiler;
        result.parserScope = parserScope;
        return result;
    }
};

struct IntrinsicContext : intrinsic::Context {
    Compiler* compiler{};

    IntrinsicContext(execution::Context& context, const instance::Scope* executionScope)
        : intrinsic::Context{context.parserScope, executionScope}
        , compiler(context.compiler) {}

    auto parse(const parser::BlockLiteral& block, instance::Scope* scope) const -> parser::Block override {
        return compiler->parseBlock(block, scope);
    }

    void report(diagnostic::Diagnostic diagnostic) override { compiler->reportDiagnostic(std::move(diagnostic)); }
};

struct Machine {
    static void runCall(const parser::Call& call, const Context& context) {
        auto tmpContext = storeTemporaryResults(call, context);
        auto callContext = storeArguments(call, tmpContext);

        runFunction(*call.function, callContext);
    }

    static void runBlock(const parser::Block& block, const Context& context) {
        auto blockContext = context.createCall();
        runFunctionBlock(block, blockContext);
    }

private:
    static void runNode(const parser::Expression& node, Context& context) {
        node.visit(
            [&](const parser::Block& block) { runFunctionBlock(block, context); },
            [&](const parser::Call& call) { runCall(call, context); },
            [&](const parser::IntrinsicCall& intrinsic) { runIntrinsic(intrinsic, context); },
            [&](const parser::ParameterReference&) {},
            [&](const parser::VariableReference&) {},
            [&](const parser::NameTypeValueReference&) {},
            [&](const parser::VariableInit& var) { initVariable(var, context); },
            [&](const parser::ModuleReference&) {},
            [&](const parser::NameTypeValueTuple& typed) { runTyped(typed, context); },
            [&](const parser::TypeReference&) {},
            [&](const parser::Value&) {});
    }

    static void initVariable(const parser::VariableInit& var, Context& context) {
        auto memory = context[&var.variable->typed];
        if (var.nodes.size() == 1) {
            for (const auto& node : var.nodes) {
                storeNode(node, context, memory);
            }
        }
    }

    static void runFunctionBlock(const parser::Block& block, Context& context) {
        auto frameSize = variablesInBlockSize(block.expressions);
        auto frameData = context.compiler->stack.allocate(frameSize);

        auto nested = context.createNested();
        nested.localBase = frameData.get();
        layoutVariables(block.expressions, nested);

        for (const auto& node : block.expressions) {
            runNode(node, nested);
        }
    }

    static void runIntrinsic(const parser::IntrinsicCall& intrinsic, Context& context) {
        Byte* memory = context.parent->localBase; // arguments
        auto intrinsicContext = IntrinsicContext{context, nullptr};
        intrinsic.exec(memory, &intrinsicContext);
    }

    static void runFunction(const instance::Function& function, Context& context) {
        runFunctionBlock(function.body.block, context);
    }

    static void runTyped(const parser::NameTypeValueTuple& typed, Context& context) {
        for (const auto& entry : typed.tuple) {
            runNode(entry.value.value(), context);
        }
    }

    static auto variablesInBlockSize(const parser::Expressions& nodes) -> size_t {
        auto sum = 0u;
        for (auto& n : nodes) {
            n.visitSome([&](const parser::VariableInit& var) { //
                sum += typeExpressionSize(var.variable->typed.type);
            });
        }
        return sum;
    }

    static auto argumentsSize(const instance::Function& fun) -> size_t {
        auto sum = 0u;
        for (auto* param : fun.parameters) {
            sum += argumentSize(*param);
        }
        return sum;
    }
    static auto argumentSize(const instance::Parameter& arg) -> size_t {
        using namespace instance;
        if (arg.flags.any(ParameterFlag::splatted)) {
            return 8; // TODO(arBmind): sizeof(Array)
        }
        if (arg.flags.any(ParameterFlag::assignable)) {
            return sizeof(void*); // passed as pointer
        }
        return typeExpressionSize(arg.typed.type);
    }

    static auto typeExpressionSize(const parser::TypeView& type) -> size_t { return type->size; }

    static auto temporaryResultSize(const parser::Call& call) -> size_t {
        auto size = size_t{};
        const auto& fun = *call.function;
        for (auto* funParam : fun.parameters) {
            auto& parameter = *funParam;
            if (parameter.side != instance::ParameterSide::result) continue;
            if (!parameter.init.empty()) continue;
            if (auto* assign = findAssign(call.arguments, parameter); assign != nullptr) continue;
            size += parameter.typed.type->size;
        }
        return size;
    }

    static auto storeTemporaryResults(const parser::Call& call, const Context& context) -> Context {
        auto tmpSize = temporaryResultSize(call);
        auto tmpData = context.compiler->stack.allocate(tmpSize);
        auto tmpContext = context.createNested();

        auto memory = tmpData.get();
        const auto& fun = *call.function;
        for (auto* funParam : fun.parameters) {
            auto& parameter = *funParam;
            if (parameter.side != instance::ParameterSide::result) continue;
            if (!parameter.init.empty()) continue;
            if (auto* assign = findAssign(call.arguments, parameter); assign != nullptr) continue;

            tmpContext.localFrame.insert(&parameter.typed, memory);
            parameter.typed.type->constructFunc(memory);
            memory += parameter.typed.type->size;
        }
        return tmpContext;
    }

    static auto storeArguments(const parser::Call& call, const Context& context) -> Context {
        auto stackSize = argumentsSize(*call.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto callContext = context.createCall();
        auto memory = callContext.localBase = stackData.get();

        const auto& fun = *call.function;
        for (auto* funParam : fun.parameters) {
            auto& parameter = *funParam;
            assignArgument(call, parameter, callContext, memory);

            callContext.localFrame.insert(&parameter.typed, memory);
            memory += argumentSize(parameter);
        }
        return callContext;
    }

    static void assignArgument(
        const parser::Call& call, //
        const instance::Parameter& parameter,
        Context& context,
        Byte* memory) {

        if (auto* assign = findAssign(call.arguments, parameter); assign != nullptr) {
            storeArgument(*context.caller, memory, parameter, assign->values);
        }
        else if (!parameter.init.empty()) {
            storeArgument(*context.caller, memory, parameter, parameter.init);
        }
        else if (parameter.side == instance::ParameterSide::result) {
            auto tmpMemory = (*context.caller)[&parameter.typed];
            storeResultAt(memory, parameter, tmpMemory);
        }
    }

    static void storeArgumentsAt(const parser::Call& call, Context& context, Byte* result) {
        Byte* memory = context.localBase;
        const auto& fun = *call.function;
        // assert(call.arguments sufficient & valid)
        for (auto* funParam : fun.parameters) {
            context.localFrame.insert(&funParam->typed, memory);
            if (funParam->side == instance::ParameterSide::result) {
                storeResultAt(memory, *funParam, result);
            }
            else {
                assignArgument(call, *funParam, context, memory);
            }
            memory += argumentSize(*funParam);
        }
    }

    static void layoutVariables(const parser::Expressions& nodes, Context& context) {
        Byte* memory = context.localBase;
        for (const auto& node : nodes) {
            node.visitSome([&](const parser::VariableInit& var) { //
                context.localFrame.insert(&var.variable->typed, memory);
                memory += typeExpressionSize(var.variable->typed.type);
            });
        }
    }

    static auto findAssign(const parser::ArgumentAssignments& assignments, const instance::Parameter& param)
        -> const parser::ArgumentAssignment* {
        for (const auto& assign : assignments) {
            if (assign.parameter == &param) return &assign;
        }
        return nullptr;
    }

    static void storeArgument(
        const Context& context, //
        Byte* memory,
        const instance::Parameter& param,
        const parser::Expressions& nodes) {

        using namespace instance;
        if (param.flags.any(ParameterFlag::splatted)) {
            assert(false); // TODO(arBmind)
            return;
        }
        if (param.flags.any(ParameterFlag::assignable)) {
            assert(nodes.size() == 1);
            nodes[0].visit(
                [&](const parser::VariableReference& var) { storeTypedAddress(var.variable->typed, context, memory); },
                [&](const parser::ParameterReference& param) {
                    storeTypedAddress(param.parameter->typed, context, memory);
                },
                [&](const parser::Value& value) { storeValueAddress(value, memory); },
                [&](const auto&) { assert(false); });
            return;
        }
        for (const auto& node : nodes) {
            storeNode(node, context, memory);
        }
    }

    static void storeResultAt(Byte* memory, const instance::Parameter& param, Byte* result) {
        (void)param;
        reinterpret_cast<void*&>(*memory) = result;
    }

    static void storeNode(const parser::Expression& node, const Context& context, Byte* memory) {
        node.visit(
            [&](const parser::Block&) { assert(false); },
            [&](const parser::Call& call) { storeCallResult(call, context, memory); },
            [&](const parser::IntrinsicCall&) { assert(false); },
            [&](const parser::ParameterReference& param) { storeTypedValue(param.parameter->typed, context, memory); },
            [&](const parser::VariableReference& var) { storeTypedValue(var.variable->typed, context, memory); },
            [&](const parser::NameTypeValueReference& ref) {
                if (ref.nameTypeValue && ref.nameTypeValue->value)
                    storeNode(ref.nameTypeValue->value.value(), context, memory);
                else
                    assert(false);
            },
            [&](const parser::VariableInit&) { assert(false); },
            [&](const parser::ModuleReference&) {},
            [&](const parser::NameTypeValueTuple& tuple) { storeTupleCopy(tuple, memory); },
            [&](const parser::TypeReference&) { assert(false); },
            [&](const parser::Value& value) { storeValueCopy(value, memory); });
    }

    static void storeCallResult(const parser::Call& call, const Context& context, Byte* memory) {
        auto stackSize = argumentsSize(*call.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto callContext = context.createCall();
        callContext.localBase = stackData.get();
        storeArgumentsAt(call, callContext, memory);

        runFunction(*call.function, callContext);
    }

    static void storeTypedAddress(const instance::Typed& typed, const Context& context, Byte* memory) {
        reinterpret_cast<void*&>(*memory) = context[&typed];
    }

    static void storeValueAddress(const parser::Value& value, Byte* memory) {
        reinterpret_cast<const void*&>(*memory) = value.data(); // store pointer to real instance
    }

    static void storeTypedValue(const instance::Typed& typed, const Context& context, Byte* memory) {
        auto* source = context[&typed];
        assert(source);
        cloneTypeInto(typed.type, memory, source);
    }

    static void storeTupleCopy(const parser::NameTypeValueTuple& tuple, Byte* memory) {
        new (memory) parser::NameTypeValueTuple(tuple);
    }

    static void storeValueCopy(const parser::Value& value, Byte* memory) {
        cloneTypeInto(value.type(), memory, reinterpret_cast<const Byte*>(value.data()));
    }

    static void cloneTypeInto(const parser::TypeView& type, Byte* dest, const Byte* source) {
        type->cloneFunc(dest, source);
    }
};

} // namespace execution
