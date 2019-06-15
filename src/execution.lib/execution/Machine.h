#pragma once
#include "execution/Frame.h"
#include "execution/Stack.h"

#include "parser/Tree.h"

#include "instance/Function.h"
#include "instance/IntrinsicContext.h"
#include "instance/Scope.h"
#include "instance/Variable.h"

#include <cassert>
#include <functional>

namespace execution {

using ParseBlock = std::function<parser::Block(const nesting::BlockLiteral& block, instance::Scope* scope)>;
using ReportDiagnositc = std::function<void(diagnostic::Diagnostic)>;

struct Compiler {
    Stack stack{}; // stack allocator
    ParseBlock parseBlock{};
    ReportDiagnositc reportDiagnostic = [](diagnostic::Diagnostic) {};
};

struct Context {
    Context* parent{};
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

    auto createCall() const -> Context {
        auto result = Context{};
        result.compiler = compiler;
        result.caller = this;
        result.parserScope = parserScope;
        return result;
    }
    auto createNested() & -> Context {
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
        auto stackSize = argumentsSize(*call.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto callContext = context.createCall();
        callContext.localBase = stackData.get();
        storeArguments(call, callContext);

        runFunction(*call.function, callContext);
    }

    static void runBlock(const parser::Block& block, const Context& context) {
        auto blockContext = context.createCall();
        runFunctionBlock(block, blockContext);
    }

private:
    static void runNode(const parser::Node& node, Context& context) {
        node.visit(
            [&](const parser::Block& block) { runFunctionBlock(block, context); },
            [&](const parser::Call& call) { runCall(call, context); },
            [&](const parser::IntrinsicCall& intrinsic) { runIntrinsic(intrinsic, context); },
            [&](const parser::ArgumentReference&) {},
            [&](const parser::VariableReference&) {},
            [&](const parser::VariableInit& var) { initVariable(var, context); },
            [&](const parser::ModuleReference&) {},
            [&](const parser::NameTypeValueTuple& typed) { runTyped(typed, context); },
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
        auto frameSize = variablesInBlockSize(block.nodes);
        auto frameData = context.compiler->stack.allocate(frameSize);

        auto nested = context.createNested();
        nested.localBase = frameData.get();
        layoutVariables(block.nodes, nested);

        for (const auto& node : block.nodes) {
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

    static auto variablesInBlockSize(const parser::Nodes& nodes) -> size_t {
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
        for (auto* a : fun.arguments) {
            sum += argumentSize(*a);
        }
        return sum;
    }
    static auto argumentSize(const instance::Argument& arg) -> size_t {
        using namespace instance;
        if (arg.flags.any(ArgumentFlag::splatted)) {
            return 8; // TODO(arBmind): sizeof(Array)
        }
        if (arg.flags.any(ArgumentFlag::assignable)) {
            return sizeof(void*); // passed as pointer
        }
        return typeExpressionSize(arg.typed.type);
    }

    static auto typeExpressionSize(const parser::TypeExpression& type) -> size_t {
        using namespace parser;
        return type.visit(
            [](const Auto&) -> size_t { return 0u; },
            [](const Array& a) -> size_t { return a.count * typeExpressionSize(*a.element); },
            [](const TypeInstance& i) -> size_t { return i.concrete->size; },
            [](const Pointer&) -> size_t { return sizeof(void*); });
    }

    static void storeArguments(const parser::Call& call, Context& context) {
        Byte* memory = context.localBase;
        const auto& fun = *call.function;
        // assert(call.arguments sufficient & valid)
        for (auto* funArg : fun.arguments) {
            context.localFrame.insert(&funArg->typed, memory);
            assignArgument(call, *funArg, context, memory);
            memory += argumentSize(*funArg);
        }
    }

    static void assignArgument(
        const parser::Call& call, //
        const instance::Argument& argument,
        Context& context,
        Byte* memory) {

        auto* assign = findAssign(call.arguments, argument);
        if (assign != nullptr) {
            storeArgument(*context.caller, memory, argument, assign->values);
        }
        else {
            storeArgument(*context.caller, memory, argument, argument.init);
        }
    }

    static void storeArgumentsAt(const parser::Call& call, Context& context, Byte* result) {
        Byte* memory = context.localBase;
        const auto& fun = *call.function;
        // assert(call.arguments sufficient & valid)
        for (auto* funArg : fun.arguments) {
            context.localFrame.insert(&funArg->typed, memory);
            if (funArg->side == instance::ArgumentSide::result) {
                storeResultAt(memory, *funArg, result);
            }
            else {
                assignArgument(call, *funArg, context, memory);
            }
            memory += argumentSize(*funArg);
        }
    }

    static void layoutVariables(const parser::Nodes& nodes, Context& context) {
        Byte* memory = context.localBase;
        for (const auto& node : nodes) {
            node.visitSome([&](const parser::VariableInit& var) { //
                context.localFrame.insert(&var.variable->typed, memory);
                memory += typeExpressionSize(var.variable->typed.type);
            });
        }
    }

    static auto findAssign(const parser::ArgumentAssignments& assignments, const instance::Argument& arg)
        -> const parser::ArgumentAssignment* {
        for (const auto& assign : assignments) {
            if (assign.argument == &arg) return &assign;
        }
        return nullptr;
    }

    static void storeArgument(
        const Context& context, //
        Byte* memory,
        const instance::Argument& arg,
        const parser::Nodes& nodes) {

        using namespace instance;
        if (arg.flags.any(ArgumentFlag::splatted)) {
            assert(false); // TODO(arBmind)
            return;
        }
        if (arg.flags.any(ArgumentFlag::assignable)) {
            assert(nodes.size() == 1);
            nodes[0].visit(
                [&](const parser::VariableReference& var) { storeTypedAddress(var.variable->typed, context, memory); },
                [&](const parser::ArgumentReference& arg) { storeTypedAddress(arg.argument->typed, context, memory); },
                [&](const parser::Value& value) { storeValueAddress(value, memory); },
                [&](const auto&) { assert(false); });
            return;
        }
        for (const auto& node : nodes) {
            storeNode(node, context, memory);
        }
    }

    static void storeResultAt(Byte* memory, const instance::Argument& arg, Byte* result) {
        (void)arg;
        reinterpret_cast<void*&>(*memory) = result;
    }

    static void storeNode(const parser::Node& node, const Context& context, Byte* memory) {
        node.visit(
            [&](const parser::Block&) { assert(false); },
            [&](const parser::Call& call) { storeCallResult(call, context, memory); },
            [&](const parser::IntrinsicCall&) { assert(false); },
            [&](const parser::ArgumentReference& arg) { storeTypedValue(arg.argument->typed, context, memory); },
            [&](const parser::VariableReference& var) { storeTypedValue(var.variable->typed, context, memory); },
            [&](const parser::VariableInit&) { assert(false); },
            [&](const parser::ModuleReference&) {},
            [&](const parser::NameTypeValueTuple& tuple) { storeTupleCopy(tuple, memory); },
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

    static void cloneTypeInto(const parser::TypeExpression& type, Byte* dest, const Byte* source) {
        type.visit(
            [&](const parser::TypeInstance& i) { i.concrete->clone(dest, source); },
            [&](const parser::Pointer& p) {
                *reinterpret_cast<void**>(dest) = *reinterpret_cast<void* const*>(source);
            },
            [](auto) { abort(); });
    }
};

} // namespace execution
