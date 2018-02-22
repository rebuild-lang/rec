#pragma once
#include "execution/Scope.h"
#include "execution/Stack.h"

#include "expression/Tree.h"

#include "instance/Function.h"
#include "instance/Variable.h"

#include <cassert>

namespace execution {

namespace expression = parser::expression;

struct Compiler {
    Stack stack{}; // stack allocator
};

struct Context {
    Context* parent{};
    Compiler* compiler{};

    const Context* caller{};

    Byte* localBase{};
    LocalScope locals{};

    auto operator[](instance::TypedView typed) const& -> Byte* {
        auto addr = locals[typed];
        if (addr) return addr;
        if (parent) return (*parent)[typed];
        return nullptr;
    }

    auto createInvoke() const -> Context {
        auto result = Context{};
        result.compiler = compiler;
        result.caller = this;
        return result;
    }
    auto createNested() & -> Context {
        auto result = Context{};
        result.parent = this;
        result.compiler = compiler;
        return result;
    }
};

struct Machine {
    static void invoke(const expression::Invocation& invocation, const Context& context) {
        auto stackSize = argumentsSize(*invocation.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto invokeContext = context.createInvoke();
        invokeContext.localBase = stackData.get();
        storeArguments(invocation, invokeContext);

        runFunction(*invocation.function, invokeContext);
    }

private:
    static void runNode(const expression::Node& node, Context& context) {
        node.visit(
            [&](const expression::Block& block) { runBlock(block, context); },
            [&](const expression::Invocation& invocation) { invoke(invocation, context); },
            [&](const expression::IntrinsicInvocation& intrinsic) { runIntrinsic(intrinsic, context); },
            [&](const expression::ArgumentReference&) {},
            [&](const expression::VariableReference&) {},
            [&](const expression::VariableInit& var) { initVariable(var, context); },
            [&](const expression::NamedTuple& named) { runNamed(named, context); },
            [&](const expression::Literal&) {});
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
        auto stackSize = variablesInBlockSize(block.nodes);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto nested = context.createNested();
        nested.localBase = stackData.get();
        layoutVariables(block.nodes, nested);

        for (const auto& node : block.nodes) {
            runNode(node, nested);
        }
    }

    static void runIntrinsic(const expression::IntrinsicInvocation& intrinsic, Context& context) {
        Byte* memory = context.parent->localBase; // arguments
        intrinsic.exec(memory);
    }

    static void runFunction(const instance::Function& function, Context& context) {
        // TODO: make space for variables
        runBlock(function.body.block, context);
    }

    static void runNamed(const expression::NamedTuple& named, Context& context) {
        for (const auto& entry : named.tuple) {
            runNode(entry.node, context);
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
        // TODO: this depends on the calling convention
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

    static auto typeExpressionSize(const instance::type::Expression& type) -> size_t {
        using namespace instance;
        return type.visit(
            [](const type::Auto&) -> size_t { return 0u; },
            [](const type::Array& a) -> size_t { return a.count * typeExpressionSize(*a.element); },
            [](const type::Instance& i) -> size_t { return i.concrete->size; },
            [](const type::Pointer&) -> size_t { return sizeof(void*); });
    }

    static void storeArguments(const expression::Invocation& inv, Context& context) {
        Byte* memory = context.localBase;
        const auto& fun = *inv.function;
        // assert(invocation.arguments sufficient & valid)
        for (const auto& funArg : fun.arguments) {
            context.locals.insert(&funArg.typed, memory);
            assignArgument(inv, funArg, context, memory);
            memory += argumentSize(funArg);
        }
    }

    static void assignArgument(
        const expression::Invocation& inv, //
        const instance::Argument& funArg,
        Context& context,
        Byte* memory) {

        auto* assign = findAssign(inv.arguments, funArg);
        if (assign != nullptr) {
            storeArgument(*context.caller, memory, funArg, assign->values);
        }
        else {
            storeArgument(*context.caller, memory, funArg, funArg.init);
        }
    }

    static void storeArgumentsAt(const expression::Invocation& inv, Context& context, Byte* result) {
        Byte* memory = context.localBase;
        const auto& fun = *inv.function;
        // assert(invocation.arguments sufficient & valid)
        for (const auto& funArg : fun.arguments) {
            context.locals.insert(&funArg.typed, memory);
            if (funArg.side == instance::ArgumentSide::result) {
                storeResultAt(memory, funArg, result);
            }
            else {
                assignArgument(inv, funArg, context, memory);
            }
            memory += argumentSize(funArg);
        }
    }

    static void layoutVariables(const expression::Nodes& nodes, Context& context) {
        Byte* memory = context.localBase;
        for (const auto& node : nodes) {
            node.visitSome([&](const expression::VariableInit& var) { //
                context.locals.insert(&var.variable->typed, memory);
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
            assert(nodes.size() == 1);
            nodes[0].visit(
                [&](const expression::VariableReference& var) { storeAddress(var.variable->typed, context, memory); },
                [&](const expression::ArgumentReference& arg) { storeAddress(arg.argument->typed, context, memory); },
                [&](const auto&) { assert(false); });
            return;
        }
        for (const auto& node : nodes) {
            storeNode(node, context, memory);
        }
    }

    static void storeResultAt(Byte* memory, const instance::Argument& arg, Byte* result) {
        reinterpret_cast<void*&>(*memory) = result;
    }

    static void storeNode(const expression::Node& node, const Context& context, Byte* memory) {
        node.visit(
            [&](const expression::Block&) { assert(false); },
            [&](const expression::Invocation& invocation) { storeInvocationResult(invocation, context, memory); },
            [&](const expression::IntrinsicInvocation&) { assert(false); },
            [&](const expression::ArgumentReference& arg) { storeValue(arg.argument->typed, context, memory); },
            [&](const expression::VariableReference& var) { storeValue(var.variable->typed, context, memory); },
            [&](const expression::VariableInit&) { assert(false); },
            [&](const expression::NamedTuple&) {},
            [&](const expression::Literal& literal) { storeLiteral(literal, memory); });
    }

    static void storeInvocationResult(const expression::Invocation& invocation, const Context& context, Byte* memory) {
        auto stackSize = argumentsSize(*invocation.function);
        auto stackData = context.compiler->stack.allocate(stackSize);

        auto invokeContext = context.createInvoke();
        invokeContext.localBase = stackData.get();
        storeArgumentsAt(invocation, invokeContext, memory);

        runFunction(*invocation.function, invokeContext);
    }

    static void storeAddress(const instance::Typed& typed, const Context& context, Byte* memory) {
        reinterpret_cast<void*&>(*memory) = context[&typed];
    }

    static void storeLiteral(const expression::Literal& literal, Byte* memory) {
        literal.value.visit([&](const auto& lit) {
            reinterpret_cast<const void*&>(*memory) = &lit; // store pointer to real instance
        });
    }

    static void storeValue(const instance::Typed& typed, const Context& context, Byte* memory) {
        // TODO: allow clone() method or ensure moving
        memcpy(memory, context[&typed], typeExpressionSize(typed.type));
    }
};

} // namespace execution
