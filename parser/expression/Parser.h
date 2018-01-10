#pragma once
#include "Tree.h"

#include "LineView.h"

#include "instance/ScopeLookup.h"

namespace parser::expression {

using Scope = instance::Scope;
using Function = instance::Function;
using FunctionView = instance::FunctionView;
using BlockLiteral = parser::block::BlockLiteral;

struct Parser {
    static auto parse(const BlockLiteral &block, const Scope &parentScope) -> Block { //
        auto localScope = Scope{&parentScope};
        return parseInto(block, localScope);
    }

    static auto parseInto(const BlockLiteral &blockLiteral, Scope &scope) -> Block { //
        auto block = Block{};
        for (const auto &line : blockLiteral.lines) {
            // TODO: split operators on the line
            auto it = BlockLineView(&line);
            if (it) {
                auto expr = parseTuple(it, scope);
                if (!expr.tuple.empty()) {
                    if (expr.tuple.size() == 1 && expr.tuple.front().name.isEmpty()) {
                        // no reason to keep the tuple around, unwrap it
                        block.nodes.emplace_back(std::move(expr).tuple.front().node);
                    }
                    else {
                        block.nodes.emplace_back(std::move(expr));
                    }
                }
                if (it) {
                    // TODO: report remaining tokens on line
                    // handling: ignore / maybe try to parse?
                }
            }
        }
        return block;
    }

private:
    static auto parseTuple(BlockLineView &it, Scope &scope) -> NamedTuple {
        auto tuple = NamedTuple{};
        if (!it) return tuple;
        auto withBrackets = it.current().oneOf<block::BracketOpen>();
        if (withBrackets) ++it;
        parseTupleInto(tuple, it, scope);
        if (withBrackets) {
            if (!it) {
                // error: missing closing bracket
            }
            else if (!it.current().oneOf<block::BracketClose>()) {
                // error: missing closing bracket
            }
            else {
                ++it;
            }
        }
        return tuple;
    }

    static void parseTupleInto(NamedTuple &tuple, BlockLineView &it, Scope &scope) {
        while (it) {
            auto opt = parseSingleNamed(it, scope);
            if (opt) {
                tuple.tuple.push_back(std::move(opt).value());
            }
            auto r = parseOptionalComma(it);
            if (r == ParseOptions::finish_single) break;
        }
    }

    enum class ParseOptions {
        continue_single,
        finish_single,
    };

    static ParseOptions parseOptionalComma(BlockLineView &it) {
        if (!it) return ParseOptions::finish_single;
        if (it.current().oneOf<block::CommaSeparator>()) {
            ++it; // skip optional comma
            if (!it) return ParseOptions::finish_single;
        }
        if (it.current().oneOf<block::BracketClose>()) return ParseOptions::finish_single;
        return ParseOptions::continue_single;
    }

    static auto parseSingleNamed(BlockLineView &it, Scope &scope) -> OptNamed {
        auto name = Name{}; // parseTupleName(it);
        auto expr = parseSingle(it, scope);
        if (expr) {
            return Named{std::move(name), std::move(expr).value()};
        }
        else if (!name.isEmpty()) {
            // TODO: named void?
        }
        return {};
    }

    static auto parseSingle(BlockLineView &it, Scope &scope) -> OptNode {
        auto result = OptNode{};
        while (it) {
            auto opt = parseStep(result, it, scope);
            if (opt == ParseOptions::finish_single) break;
        }
        return result;
    }

    static auto parseStep(OptNode &result, BlockLineView &it, Scope &scope) -> ParseOptions {
        return it.current().data.visit(
            [](const block::CommaSeparator &) { return ParseOptions::finish_single; },
            [](const block::BracketClose &) { return ParseOptions::finish_single; },
            [&](const block::BracketOpen &) {
                if (result) return ParseOptions::finish_single;
                auto tuple = parseTuple(it, scope);
                result = OptNode{std::move(tuple)};
                return ParseOptions::continue_single;
            },
            [&](const block::IdentifierLiteral &id) {
                const auto &instance = scope[it.current().range.text];
                if (!instance) {
                    if (result) return ParseOptions::finish_single;
                    result = OptNode{Literal{id, it.current().range}};
                    ++it;
                    return ParseOptions::continue_single;
                }
                return parseInstance(result, *instance, it, scope);
            },
            [&](const block::StringLiteral &s) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{Literal{s, it.current().range}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const block::NumberLiteral &n) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{Literal{n, it.current().range}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const block::BlockLiteral &b) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{Literal{b, it.current().range}};
                ++it;
                return ParseOptions::continue_single;
            },
            [](const auto &) { return ParseOptions::finish_single; });
    }

    static auto parseInstance( //
        OptNode &result,
        const instance::Node &instance,
        BlockLineView &it,
        Scope &scope) -> ParseOptions {
        return instance.visit(
            [&](const instance::Variable &var) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{VariableReference{&var}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::Argument &arg) {
                if (result) return ParseOptions::finish_single;
                (void)arg;
                // result = OptNode{ArgumentReference{&arg}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::Function &fun) {
                ++it;
                return parseInvocation(result, fun, it, scope);
            },
            [&](const instance::Type &type) {
                if (result) return ParseOptions::finish_single;
                (void)type;
                // result = OptNode{TypeReference{&type}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::Module &mod) {
                if (result) return ParseOptions::finish_single;
                (void)mod;
                // result = OptNode{ModuleReference{&mod}};
                ++it;
                return ParseOptions::continue_single;
            });
        // TODO: add modules
        // TODO: add overloads
    }

    static bool canImplicitConvertToType(NodeView node, instance::TypeView type) {
        // TODO:
        // I guess we need a scope here
        (void)node;
        (void)type;
        return true;
    }

    struct OverloadSet {
        struct Overload {
            using This = Overload;
            bool active{true};
            bool complete{false};
            FunctionView function{};
            BlockLineView it{};
            ArgumentAssignments rightArgs{};
            size_t nextArg{};

            Overload() = default;
            Overload(const Function &function)
                : function(&function) {}

            void retireLeft(const NamedTupleView &left) {
                auto o = 0u, t = 0u;
                auto la = function->leftArguments();
                for (const NamedNodeView &named : left.tuple) {
                    if (!named.name.isEmpty()) {
                        auto optArg = function->lookupArgument(named.name);
                        if (optArg) {
                            const instance::Argument &arg = optArg.value();
                            if (arg.side == instance::ArgumentSide::left //
                                && canImplicitConvertToType(named.node, arg.type)) {
                                t++;
                                continue;
                            }
                            // side does not match
                            // type does not match
                        }
                        // name not found
                    }
                    else if (o < la.size()) {
                        const auto &arg = la[o];
                        if (arg.side == instance::ArgumentSide::left //
                            && canImplicitConvertToType(named.node, arg.type)) {
                            o++;
                            continue;
                        }
                        // side does not match
                        // type does not match
                    }
                    // index out of range
                    active = false;
                    return;
                }
                if (o + t == la.size()) return;
                // not right count
                active = false;
            }

            auto arg() const -> const instance::Argument & { return function->rightArguments()[nextArg]; }
        };
        using Overloads = std::vector<Overload>;

        OverloadSet(const Function &fun) {
            vec.emplace_back(fun);
            activeCount = 1;
        }
        // TODO: allow multiple overloads

        void retireLeft(const OptNode &left) {
            auto leftView = left ? left.value().holds<NamedTuple>() ? NamedTupleView{left.value().get<NamedTuple>()}
                                                                    : NamedTupleView{left.value()}
                                 : NamedTupleView{};
            for (auto &o : vec) o.retireLeft(leftView);
            update();
        }

        void setupIt(BlockLineView &it) {
            for (auto &o : vec) o.it = it;
        }

        auto active() const & -> meta::VectorRange<const Overload> { return {vec.begin(), vec.begin() + activeCount}; }
        auto active() & -> meta::VectorRange<Overload> { return {vec.begin(), vec.begin() + activeCount}; }

        void update() {
            auto it = std::stable_partition(
                vec.begin(), vec.begin() + activeCount, [](const Overload &o) { return o.active; });
            activeCount = std::distance(vec.begin(), it);
        }
        auto finish() & -> meta::VectorRange<Overload> {
            auto it = std::stable_partition(vec.begin(), vec.end(), [](const Overload &o) { return o.complete; });
            return {vec.begin(), it};
        }

    private:
        Overloads vec{};
        size_t activeCount{};
    };

    static auto parseInvocation( //
        OptNode &left,
        const Function &fun,
        BlockLineView &it,
        Scope &scope) -> ParseOptions { //

        auto os = OverloadSet{fun};
        os.retireLeft(left);
        if (!os.active().empty() && it) {
            parseArguments(os, it, scope);
            auto completed = os.finish();
            if (completed.size() == 1) {
                auto &o = completed.front();
                it = o.it;
                // TODO: add compile time execution
                auto inv = Invocation{};
                inv.function = o.function;
                // TODO: assign left arguments
                inv.arguments = o.rightArgs;
                left = OptNode{{inv}};
                return ParseOptions::continue_single;
            }
        }
        if (left) return ParseOptions::finish_single;
        // left = OptNode{FunctionReference{fun}};
        return ParseOptions::continue_single;
    }

    static void parseArguments(OverloadSet &os, BlockLineView &it, Scope &scope) {
        auto withBrackets = it.current().oneOf<block::BracketOpen>();
        if (withBrackets) ++it;
        parseArgumentsWithout(os, it, scope);
        if (withBrackets) {
            if (!it) {
                // error: missing closing bracket
            }
            else if (!it.current().oneOf<block::BracketClose>()) {
                // error: missing closing bracket
            }
            else {
                ++it;
            }
        }
    }

    static void parseArgumentsWithout(OverloadSet &os, BlockLineView &it, Scope &scope) {
        os.setupIt(it);
        while (!os.active().empty()) {
            // auto baseIt = it;
            // TODO: optimize for no custom parser case!
            for (auto &o : os.active()) {
                const auto &a = o.arg();
                // auto p = nullptr; // parserForType(a.type);
                auto optNamed = parseSingleNamed(o.it, scope); // : p.parse(o.it, scope);
                if (optNamed) {
                    Named &named = optNamed.value();
                    if (!named.name.isEmpty()) {
                        auto optArg = o.function->lookupArgument(named.name);
                        if (optArg) {
                            const instance::Argument &arg = optArg.value();
                            if (canImplicitConvertToType(&named.node, a.type)) {
                                auto as = ArgumentAssignment{};
                                as.argument = &arg;
                                as.values = {std::move(optNamed).value().node};
                                o.rightArgs.push_back(std::move(as));
                            }
                            else {
                                // type does not match
                            }
                        }
                        else {
                            // name not found
                        }
                    }
                    else {
                        if (canImplicitConvertToType(&named.node, a.type)) {
                            auto as = ArgumentAssignment{};
                            as.argument = &a;
                            as.values = {std::move(optNamed).value().node};
                            o.rightArgs.push_back(std::move(as));
                            o.nextArg++;
                        }
                        else {
                            // type does not match
                        }
                    }
                }
                else {
                    // no value
                }

                if (o.nextArg == o.function->rightArguments().size()) {
                    o.complete = true;
                    o.active = false;
                }
                else {
                    auto r = parseOptionalComma(o.it);
                    if (r == ParseOptions::finish_single) {
                        o.active = false;
                    }
                }
            }
            os.update();
        }
    }
};

} // namespace parser::expression
