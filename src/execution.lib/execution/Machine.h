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

using ParseBlock = std::function<parser::Block(const nesting::BlockLiteral& block, const instance::ScopePtr& scope)>;
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
    instance::ScopePtr parserScope{};

    Byte* localBase{};
    LocalFrame localFrame{};

    auto byVariable(instance::VariableView var) const& -> Byte* {
        auto addr = localFrame.byVariable(var);
        if (addr) return addr;
        if (parent) return parent->byVariable(var);
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

struct IntrinsicContext : intrinsic::ContextInterface {
    Compiler* compiler{};

    IntrinsicContext(execution::Context& context, const instance::ScopePtr& executionScope)
        : intrinsic::ContextInterface{context.parserScope, executionScope}
        , compiler(context.compiler) {}

    auto parse(const parser::BlockLiteral& block, const instance::ScopePtr& scope) const -> parser::Block override {
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
    static void runBlockExpr(const parser::BlockExpr& expr, Context& context) {
        expr.visit(
            [&](const parser::Block& block) { runFunctionBlock(block, context); },
            [&](const parser::Call& call) { runCall(call, context); },
            [&](const parser::VariableReference&) {},
            [&](const parser::NameTypeValueReference&) {},
            [&](const parser::VariableInit& varInit) { initVariable(varInit, context); },
            [&](const parser::ModuleInit& modInit) { (void)modInit; }, // TODO(arBmind)
            [&](const parser::ModuleReference&) {},
            [&](const parser::NameTypeValueTuple& ntvTuple) { runTuple(ntvTuple, context); },
            [&](const parser::TypeReference&) {},
            [&](const parser::Value&) {},
            [](const parser::VecOfPartiallyParsed&) {});
    }

    static void initVariable(const parser::VariableInit& var, Context& context) {
        auto memory = context.byVariable(var.variable);
        if (var.nodes.size() == 1) {
            for (const auto& node : var.nodes) {
                storeValueExpr(node, context, memory);
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
            runBlockExpr(node, nested);
        }
    }

    static void runIntrinsic(const instance::IntrinsicCall& intrinsic, Context& context) {
        Byte* memory = context.localBase; // arguments
        auto intrinsicContext = IntrinsicContext{context, nullptr};
        intrinsic.exec(memory, &intrinsicContext);
    }

    static void runFunction(const instance::Function& function, Context& context) {
        function.body.visit(
            [&](const instance::IntrinsicCall& intrinsic) { runIntrinsic(intrinsic, context); },
            [&](const instance::ParsedBlock& localBlock) { runFunctionBlock(localBlock.block, context); });
    }

    static void runTuple(const parser::NameTypeValueTuple& ntvTuple, Context& context) {
        for (const auto& entry : ntvTuple.tuple) {
            runValueExpr(entry.value.value(), context);
        }
    }
    static void runValueExpr(const parser::ValueExpr& expr, Context& context) {
        expr.visit(
            [&](const parser::Block& block) { runFunctionBlock(block, context); },
            [&](const parser::Call& call) { runCall(call, context); },
            [&](const parser::VariableReference&) {},
            [&](const parser::NameTypeValueReference&) {},
            [&](const parser::ModuleReference&) {},
            [&](const parser::NameTypeValueTuple& indexNtvs) { runTuple(indexNtvs, context); },
            [&](const parser::TypeReference&) {},
            [&](const parser::Value&) {},
            [](const parser::VecOfPartiallyParsed&) {});
    }

    static auto variablesInBlockSize(const parser::VecOfBlockExpr& nodes) -> size_t {
        auto sum = 0u;
        for (auto& n : nodes) {
            n.visitSome([&](const parser::VariableInit& var) { //
                sum += typeExpressionSize(var.variable->type);
            });
        }
        return sum;
    }

    static auto argumentsSize(const instance::Function& fun) -> size_t {
        auto sum = 0u;
        for (const auto& parameter : fun.parameters) {
            sum += parameterVariableSize(parameter.get());
        }
        return sum;
    }
    static auto parameterVariableSize(instance::ParameterView param) -> size_t {
        using namespace instance;
        if (param->flags.any(ParameterFlag::splatted)) {
            return 8; // TODO(arBmind): sizeof(Array)
        }
        if (param->flags.any(ParameterFlag::assignable)) {
            return sizeof(void*); // passed as pointer
        }
        if (param->variable->type) {
            return param->variable->type->size;
        }
        return 1; // error!
    }

    static auto typeExpressionSize(const parser::TypeView& type) -> size_t { return type->size; }

    // results are assignable, so we pass a pointer
    // but we have also have to provide the storage this pointer points to
    static auto temporaryResultSize(const parser::Call& call) -> size_t {
        auto size = size_t{};
        const auto& fun = *call.function;
        for (const auto& parameter : fun.parameters) {
            if (parameter->side != instance::ParameterSide::result) continue;
            if (!parameter->defaultValue.empty()) continue;
            if (auto* assign = findAssign(call.arguments, *parameter); assign != nullptr) continue;
            size += parameter->variable->type->size;
        }
        return size;
    }

    static auto storeTemporaryResults(const parser::Call& call, const Context& context) -> Context {
        auto tmpSize = temporaryResultSize(call);
        auto tmpData = context.compiler->stack.allocate(tmpSize);
        auto tmpContext = context.createNested();

        auto memory = tmpData.get();
        const auto& fun = *call.function;
        for (const auto& param : fun.parameters) {
            if (param->side != instance::ParameterSide::result) continue;
            if (!param->defaultValue.empty()) continue;
            if (auto* assign = findAssign(call.arguments, *param); assign != nullptr) continue;

            tmpContext.localFrame.insert(param->variable, memory);
            param->variable->type->constructFunc(memory);
            memory += parameterVariableSize(param.get());
        }
        return tmpContext;
    }

    static auto storeArguments(const parser::Call& call, const Context& context) -> Context {
        auto stackSize = argumentsSize(*call.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto callContext = context.createCall();
        auto memory = callContext.localBase = stackData.get();

        const auto& fun = *call.function;
        for (const auto& param : fun.parameters) {
            assignArgument(call, *param, callContext, memory);

            callContext.localFrame.insert(param->variable, memory);
            memory += parameterVariableSize(param.get());
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
        else if (!parameter.defaultValue.empty()) {
            storeArgument(*context.caller, memory, parameter, parameter.defaultValue);
        }
        else if (parameter.side == instance::ParameterSide::result) {
            auto tmpMemory = context.caller->byVariable(parameter.variable);
            storeResultAt(memory, parameter, tmpMemory);
        }
    }

    static void storeArgumentsAt(const parser::Call& call, Context& context, Byte* result) {
        Byte* memory = context.localBase;
        const auto& fun = *call.function;
        // assert(call.arguments sufficient & valid)
        for (auto& funParam : fun.parameters) {
            // context.localFrame.insert(&funParam, memory);
            if (funParam->side == instance::ParameterSide::result) {
                storeResultAt(memory, *funParam, result);
            }
            else {
                assignArgument(call, *funParam, context, memory);
            }
            memory += parameterVariableSize(funParam.get());
        }
    }

    static void layoutVariables(const parser::VecOfBlockExpr& nodes, Context& context) {
        Byte* memory = context.localBase;
        for (const auto& node : nodes) {
            node.visitSome([&](const parser::VariableInit& var) { //
                context.localFrame.insert(var.variable, memory);
                memory += typeExpressionSize(var.variable->type);
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
        const parser::VecOfValueExpr& nodes) {

        using namespace instance;
        if (param.flags.any(ParameterFlag::splatted)) {
            assert(false); // TODO(arBmind)
            return;
        }
        if (param.flags.any(ParameterFlag::assignable)) {
            assert(nodes.size() == 1);
            nodes[0].visit(
                [&](const parser::VariableReference& var) { storeVariableAddress(*var.variable, context, memory); },
                [&](const parser::Value& value) { storeValueAddress(value, memory); },
                [&](const auto&) { assert(false); });
            return;
        }
        for (const auto& node : nodes) {
            storeValueExpr(node, context, memory);
        }
    }

    static void storeResultAt(Byte* memory, const instance::Parameter& param, Byte* result) {
        (void)param;
        reinterpret_cast<void*&>(*memory) = result;
    }

    static void storeValueExpr(const parser::ValueExpr& node, const Context& context, Byte* memory) {
        node.visit(
            [&](const parser::Block&) { assert(false); },
            [&](const parser::Call& call) { storeCallResult(call, context, memory); },
            [&](const parser::VariableReference& var) { storeVariableValue(*var.variable, context, memory); },
            [&](const parser::NameTypeValueReference& ref) {
                if (ref.nameTypeValue && ref.nameTypeValue->value)
                    storeValueExpr(ref.nameTypeValue->value.value(), context, memory);
                else
                    assert(false);
            },
            [&](const parser::VariableInit&) { assert(false); },
            [&](const parser::ModuleReference&) {},
            [&](const parser::NameTypeValueTuple& tuple) { storeTupleCopy(tuple, memory); },
            [&](const parser::TypeReference&) { assert(false); },
            [&](const parser::Value& value) { storeValueCopy(value, memory); },
            [](const parser::VecOfPartiallyParsed&) {});
    }

    static void storeCallResult(const parser::Call& call, const Context& context, Byte* memory) {
        auto stackSize = argumentsSize(*call.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto callContext = context.createCall();
        callContext.localBase = stackData.get();
        storeArgumentsAt(call, callContext, memory);

        runFunction(*call.function, callContext);
    }

    static void storeVariableAddress(const instance::Variable& var, const Context& context, Byte* memory) {
        reinterpret_cast<void*&>(*memory) = context.byVariable(&var);
    }

    static void storeValueAddress(const parser::Value& value, Byte* memory) {
        reinterpret_cast<const void*&>(*memory) = value.data(); // store pointer to real instance
    }

    static void storeVariableValue(const instance::Variable& var, const Context& context, Byte* memory) {
        auto* source = context.byVariable(&var);
        assert(source);
        cloneTypeInto(var.type, memory, source);
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
