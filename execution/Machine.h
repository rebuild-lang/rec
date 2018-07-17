#pragma once
#include "execution/Frame.h"
#include "execution/Stack.h"

#include "expression/Tree.h"

#include "instance/Function.h"
#include "instance/Scope.h"
#include "instance/Variable.h"

#include "intrinsic/Context.h"

#include <cassert>
#include <functional>

namespace execution {

namespace expression = parser::expression;

using ParseBlock = std::function<void(const parser::block::BlockLiteral& block, instance::Scope* scope)>;

struct Compiler {
    Stack stack{}; // stack allocator
    ParseBlock parseBlock{};
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
    ParseBlock parseBlock{};

    IntrinsicContext(execution::Context& context, const instance::Scope* executionScope)
        : parseBlock(context.compiler->parseBlock)
        , intrinsic::Context{context.parserScope, executionScope} {}

    void parse(const parser::expression::BlockLiteral& block, instance::Scope* scope) const override {
        parseBlock(block, scope);
    }
};

struct Machine {
    static void runCall(const expression::Call& call, const Context& context) {
        auto stackSize = argumentsSize(*call.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto callContext = context.createCall();
        callContext.localBase = stackData.get();
        storeArguments(call, callContext);

        runFunction(*call.function, callContext);
    }

private:
    static void runNode(const expression::Node& node, Context& context) {
        node.visit(
            [&](const expression::Block& block) { runBlock(block, context); },
            [&](const expression::Call& call) { runCall(call, context); },
            [&](const expression::IntrinsicCall& intrinsic) { runIntrinsic(intrinsic, context); },
            [&](const expression::ArgumentReference&) {},
            [&](const expression::VariableReference&) {},
            [&](const expression::VariableInit& var) { initVariable(var, context); },
            [&](const expression::ModuleReference&) {},
            [&](const expression::TypedTuple& typed) { runTyped(typed, context); },
            [&](const expression::Value&) {});
    }

    static void initVariable(const expression::VariableInit& var, Context& context) {
        auto memory = context[&var.variable->typed];
        if (var.nodes.size() == 1) {
            for (const auto& node : var.nodes) {
                storeNode(node, context, memory);
            }
        }
    }

    static void runBlock(const expression::Block& block, Context& context) {
        auto frameSize = variablesInBlockSize(block.nodes);
        auto frameData = context.compiler->stack.allocate(frameSize);

        auto nested = context.createNested();
        nested.localBase = frameData.get();
        layoutVariables(block.nodes, nested);

        for (const auto& node : block.nodes) {
            runNode(node, nested);
        }
    }

    static void runIntrinsic(const expression::IntrinsicCall& intrinsic, Context& context) {
        Byte* memory = context.parent->localBase; // arguments
        auto intrinsicContext = IntrinsicContext{context, nullptr};
        intrinsic.exec(memory, &intrinsicContext);
    }

    static void runFunction(const instance::Function& function, Context& context) {
        runBlock(function.body.block, context);
    }

    static void runTyped(const expression::TypedTuple& typed, Context& context) {
        for (const auto& entry : typed.tuple) {
            runNode(entry.value.value(), context);
        }
    }

    static auto variablesInBlockSize(const expression::Nodes& nodes) -> size_t {
        auto sum = 0u;
        for (auto& n : nodes) {
            n.visitSome([&](const expression::VariableInit& var) { //
                sum += typeExpressionSize(var.variable->typed.type);
            });
        }
        return sum;
    }

    static auto argumentsSize(const instance::Function& fun) -> size_t {
        auto sum = 0u;
        for (auto& a : fun.arguments) {
            sum += argumentSize(a);
        }
        return sum;
    }
    static auto argumentSize(const instance::Argument& arg) -> size_t {
        using namespace instance;
        if (arg.flags.any(ArgumentFlag::splatted)) {
            return 8; // TODO: sizeof(Array)
        }
        if (arg.flags.any(ArgumentFlag::assignable)) {
            return sizeof(void*); // passed as pointer
        }
        return typeExpressionSize(arg.typed.type);
    }

    static auto typeExpressionSize(const parser::expression::TypeExpression& type) -> size_t {
        using namespace parser::expression;
        return type.visit(
            [](const Auto&) -> size_t { return 0u; },
            [](const Array& a) -> size_t { return a.count * typeExpressionSize(*a.element); },
            [](const TypeInstance& i) -> size_t { return i.concrete->size; },
            [](const Pointer&) -> size_t { return sizeof(void*); });
    }

    static void storeArguments(const expression::Call& call, Context& context) {
        Byte* memory = context.localBase;
        const auto& fun = *call.function;
        // assert(call.arguments sufficient & valid)
        for (const auto& funArg : fun.arguments) {
            context.localFrame.insert(&funArg.typed, memory);
            assignArgument(call, funArg, context, memory);
            memory += argumentSize(funArg);
        }
    }

    static void assignArgument(
        const expression::Call& call, //
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

    static void storeArgumentsAt(const expression::Call& call, Context& context, Byte* result) {
        Byte* memory = context.localBase;
        const auto& fun = *call.function;
        // assert(call.arguments sufficient & valid)
        for (const auto& funArg : fun.arguments) {
            context.localFrame.insert(&funArg.typed, memory);
            if (funArg.side == instance::ArgumentSide::result) {
                storeResultAt(memory, funArg, result);
            }
            else {
                assignArgument(call, funArg, context, memory);
            }
            memory += argumentSize(funArg);
        }
    }

    static void layoutVariables(const expression::Nodes& nodes, Context& context) {
        Byte* memory = context.localBase;
        for (const auto& node : nodes) {
            node.visitSome([&](const expression::VariableInit& var) { //
                context.localFrame.insert(&var.variable->typed, memory);
                memory += typeExpressionSize(var.variable->typed.type);
            });
        }
    }

    static auto findAssign(const expression::ArgumentAssignments& assignments, const instance::Argument& arg)
        -> const expression::ArgumentAssignment* {
        for (const auto& assign : assignments) {
            if (assign.argument == &arg) return &assign;
        }
        return nullptr;
    }

    static void storeArgument(
        const Context& context, //
        Byte* memory,
        const instance::Argument& arg,
        const expression::Nodes& nodes) {

        using namespace instance;
        if (arg.flags.any(ArgumentFlag::splatted)) {
            assert(false); // TODO
            return;
        }
        if (arg.flags.any(ArgumentFlag::assignable)) {
            /*
            assert(nodes.size() == 1);
            nodes[0].visit(
                [&](const expression::VariableReference& var) { storeAddress(var.variable->typed, context, memory); },
                [&](const expression::ArgumentReference& arg) { storeAddress(arg.argument->typed, context, memory); },
                [&](const auto&) { assert(false); });
                */
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

    static void storeNode(const expression::Node& node, const Context& context, Byte* memory) {
        node.visit(
            [&](const expression::Block&) { assert(false); },
            [&](const expression::Call& call) { storeCallResult(call, context, memory); },
            [&](const expression::IntrinsicCall&) { assert(false); },
            [&](const expression::ArgumentReference& arg) { storeValue(arg.argument->typed, context, memory); },
            [&](const expression::VariableReference& var) { storeValue(var.variable->typed, context, memory); },
            [&](const expression::VariableInit&) { assert(false); },
            [&](const expression::ModuleReference&) {},
            [&](const expression::TypedTuple&) {},
            [&](const expression::Value& value) { storeLiteral(value, memory); });
    }

    static void storeCallResult(const expression::Call& call, const Context& context, Byte* memory) {
        auto stackSize = argumentsSize(*call.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto callContext = context.createCall();
        callContext.localBase = stackData.get();
        storeArgumentsAt(call, callContext, memory);

        runFunction(*call.function, callContext);
    }

    static void storeAddress(const instance::Typed& typed, const Context& context, Byte* memory) {
        reinterpret_cast<void*&>(*memory) = context[&typed];
    }

    static void storeLiteral(const expression::Value& value, Byte* memory) {
        reinterpret_cast<const void*&>(*memory) = value.data(); // store pointer to real instance
    }

    static void storeValue(const instance::Typed& typed, const Context& context, Byte* memory) {
        // TODO: allow clone() method or ensure moving
        memcpy(memory, context[&typed], typeExpressionSize(typed.type));
    }
};

} // namespace execution
