#pragma once
#include "Tree.h"

#include "LineView.h"

#include "instance/Function.h"
#include "instance/Module.h"
#include "instance/Type.h"

#include <type_traits>
#include <utility>

namespace parser::expression {

using Function = instance::Function;
using FunctionView = instance::FunctionView;
using InputBlockLiteral = parser::block::BlockLiteral;

template<class Lookup, class RunCall>
struct Context {
    Lookup lookup;
    RunCall runCall;
};
template<class Lookup, class RunCall>
auto makeContext(Lookup&& lookup, RunCall&& runCall) {
    return Context<Lookup, RunCall>{std::forward<Lookup>(lookup), std::forward<RunCall>(runCall)};
}

template<class Context>
constexpr void checkContext() noexcept {
    static_assert(
        std::is_same_v<const instance::Node*, std::invoke_result_t<decltype(Context::lookup), strings::View>>,
        "no lookup");
    static_assert(
        std::is_same_v<OptNode, std::invoke_result_t<decltype(Context::runCall), Call>>, //
        "no runCall");
}

struct Parser {
    template<class Context>
    static auto parse(const InputBlockLiteral& blockLiteral, Context context) -> Block {
        checkContext<Context>();
        auto block = Block{};
        for (const auto& line : blockLiteral.lines) {
            // TODO: split operators on the line
            auto it = BlockLineView(&line);
            if (it) {
                auto expr = parseTuple(it, context);
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
    template<class Context>
    static auto parseTuple(BlockLineView& it, Context& context) -> NamedTuple {
        auto tuple = NamedTuple{};
        if (!it) return tuple;
        auto withBrackets = it.current().oneOf<block::BracketOpen>();
        if (withBrackets) ++it;
        parseTupleInto(tuple, it, context);
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

    template<class Context>
    static void parseTupleInto(NamedTuple& tuple, BlockLineView& it, Context& context) {
        while (it) {
            auto opt = parseSingleNamed(it, context);
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

    static ParseOptions parseOptionalComma(BlockLineView& it) {
        if (!it) return ParseOptions::finish_single;
        if (it.current().oneOf<block::CommaSeparator>()) {
            ++it; // skip optional comma
            if (!it) return ParseOptions::finish_single;
        }
        if (it.current().oneOf<block::BracketClose>()) return ParseOptions::finish_single;
        return ParseOptions::continue_single;
    }

    template<class Context>
    static auto parseSingleNamed(BlockLineView& it, Context& context) -> OptNamed {
        auto name = Name{}; // parseTupleName(it);
        auto expr = parseSingle(it, context);
        if (expr) {
            return Named{std::move(name), std::move(expr).value()};
        }
        else if (!name.isEmpty()) {
            // TODO: named void?
        }
        return {};
    }

    template<class Context>
    static auto parseSingle(BlockLineView& it, Context& context) -> OptNode {
        auto result = OptNode{};
        while (it) {
            auto opt = parseStep(result, it, context);
            if (opt == ParseOptions::finish_single) break;
        }
        return result;
    }

    template<class Context>
    static auto parseStep(OptNode& result, BlockLineView& it, Context& context) -> ParseOptions {
        return it.current().data.visit(
            [](const block::CommaSeparator&) { return ParseOptions::finish_single; },
            [](const block::BracketClose&) { return ParseOptions::finish_single; },
            [&](const block::BracketOpen&) {
                if (result) return ParseOptions::finish_single;
                auto tuple = parseTuple(it, context);
                result = Node{std::move(tuple)};
                return ParseOptions::continue_single;
            },
            [&](const block::IdentifierLiteral& id) {
                const auto& instance = lookupIdentifier(it.current().range.text, result, context);
                if (!instance) {
                    if (result) return ParseOptions::finish_single;
                    result = OptNode{expression::IdentifierLiteral{id, it.current().range}};
                    ++it;
                    return ParseOptions::continue_single;
                }
                return parseInstance(result, *instance, it, context);
            },
            [&](const block::StringLiteral& s) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{StringLiteral{s, it.current().range}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const block::NumberLiteral& n) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{NumberLiteral{n, it.current().range}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const block::BlockLiteral& b) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{BlockLiteral{b, it.current().range}};
                ++it;
                return ParseOptions::continue_single;
            },
            [](const auto&) { return ParseOptions::finish_single; });
    }

    template<class Context>
    static auto lookupIdentifier(const strings::View& id, OptNode& result, Context& context) -> const instance::Node* {

        if (result.map([](const Node& n) { return n.holds<ModuleReference>(); })) {
            auto ref = result.value().get<ModuleReference>();
            result = {};
            return ref.module->locals[id];
        }
        return context.lookup(id);
    }

    template<class Context>
    static auto parseInstance( //
        OptNode& result,
        const instance::Node& instance,
        BlockLineView& it,
        Context& context) -> ParseOptions {
        return instance.visit(
            [&](const instance::Variable& var) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{VariableReference{&var}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::Argument& arg) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{ArgumentReference{&arg}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::Function& fun) {
                ++it;
                return parseCall(result, fun, it, context);
            },
            [&](const instance::Type& type) {
                if (result) return ParseOptions::finish_single;
                (void)type;
                // result = OptNode{TypeReference{&type}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::Module& mod) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{ModuleReference{&mod}};
                ++it;
                return ParseOptions::continue_single;
            });
        // TODO: add modules
        // TODO: add overloads
    }

    static bool canImplicitConvertToType(NodeView node, const parser::expression::TypeExpression& type) {
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
            Overload(const Function& function)
                : function(&function) {}

            void retireLeft(const NamedTupleView& left) {
                auto o = 0u, t = 0u;
                auto la = function->leftArguments();
                for (const NamedNodeView& named : left.tuple) {
                    if (!named.name.isEmpty()) {
                        auto optArg = function->lookupArgument(named.name);
                        if (optArg) {
                            const instance::Argument& arg = optArg.value();
                            if (arg.side == instance::ArgumentSide::left //
                                && canImplicitConvertToType(named.node, arg.typed.type)) {
                                t++;
                                continue;
                            }
                            // side does not match
                            // type does not match
                        }
                        // name not found
                    }
                    else if (o < la.size()) {
                        const auto& arg = la[o];
                        if (arg.side == instance::ArgumentSide::left //
                            && canImplicitConvertToType(named.node, arg.typed.type)) {
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

            auto arg() const -> const instance::Argument& { return function->rightArguments()[nextArg]; }
        };
        using Overloads = std::vector<Overload>;

        OverloadSet(const Function& fun) {
            vec.emplace_back(fun);
            activeCount = 1;
        }
        // TODO: allow multiple overloads

        void retireLeft(const OptNode& left) {
            auto leftView = left ? left.value().holds<NamedTuple>() ? NamedTupleView{left.value().get<NamedTuple>()}
                                                                    : NamedTupleView{left.value()}
                                 : NamedTupleView{};
            for (auto& o : vec) o.retireLeft(leftView);
            update();
        }

        void setupIt(BlockLineView& it) {
            for (auto& o : vec) o.it = it;
        }

        auto active() const& -> meta::VectorRange<const Overload> { return {vec.begin(), vec.begin() + activeCount}; }
        auto active() & -> meta::VectorRange<Overload> { return {vec.begin(), vec.begin() + activeCount}; }

        void update() {
            auto it = std::stable_partition(
                vec.begin(), vec.begin() + activeCount, [](const Overload& o) { return o.active; });
            activeCount = std::distance(vec.begin(), it);
        }
        auto finish() & -> meta::VectorRange<Overload> {
            auto it = std::stable_partition(vec.begin(), vec.end(), [](const Overload& o) { return o.complete; });
            return {vec.begin(), it};
        }

    private:
        Overloads vec{};
        size_t activeCount{};
    };

    template<class Context>
    static auto parseCall( //
        OptNode& left,
        const Function& fun,
        BlockLineView& it,
        Context& context) -> ParseOptions { //

        auto os = OverloadSet{fun};
        os.retireLeft(left);
        if (!os.active().empty() && it) {
            parseArguments(os, it, context);
            auto completed = os.finish();
            if (completed.size() == 1) {
                auto& o = completed.front();
                it = o.it;
                auto inv = [&] {
                    auto r = Call{};
                    r.function = o.function;
                    // TODO: assign left arguments
                    r.arguments = o.rightArgs;
                    return r;
                };
                left = buildCallNode(inv(), context);
                return ParseOptions::continue_single;
            }
        }
        if (left) return ParseOptions::finish_single;
        // left = OptNode{FunctionReference{fun}};
        return ParseOptions::continue_single;
    }

    template<class Context>
    static auto buildCallNode(Call&& call, Context& context) -> OptNode {
        if (call.function->flags.none(instance::FunctionFlag::run_time)) {
            return context.runCall(call);
        }
        return OptNode{{call}};
    }

    template<class Context>
    static void parseArguments(OverloadSet& os, BlockLineView& it, Context& context) {
        auto withBrackets = it.current().oneOf<block::BracketOpen>();
        if (withBrackets) ++it;
        parseArgumentsWithout(os, it, context);
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

    template<class Token>
    struct TokenVisitor {
        BlockLineView& it;
        using BlockToken = decltype(Token::token);

        constexpr auto operator()(const BlockToken& t) {
            auto result = OptNode{Token{t, it.current().range}};
            ++it;
            return result;
        }
    };
    template<class... T>
    constexpr static auto buildTokenVisitors(BlockLineView& it, meta::TypeList<T...> = {}) {
        return meta::Overloaded{TokenVisitor<T>{it}...};
    }

    static auto parseSingleToken(BlockLineView& it) -> OptNamed {
        auto name = Name{};
        auto value = it.current().data.visit(
            buildTokenVisitors<BlockLiteral, StringLiteral, NumberLiteral, IdentifierLiteral, OperatorLiteral>(it),
            [](const auto&) { return OptNode{}; });
        if (value) {
            return Named{std::move(name), std::move(value).value()};
        }
        // error
        return {};
    }

    template<class Context>
    static auto parseTypeExpression(BlockLineView& it, Context& context) -> OptTypeExpression {
        return it.current().data.visit(
            [&](const block::IdentifierLiteral&) -> OptTypeExpression {
                auto name = it.current().range.text;
                auto node = context.lookup(name);
                if (node) return parseTypeInstance(*node, it, context);
                return {};
            },
            [](const auto&) -> OptTypeExpression { // error
                return {};
            });
    }

    template<class Context>
    static auto parseTypeInstance(const instance::Node& instance, BlockLineView& it, Context& context)
        -> OptTypeExpression {
        return instance.visit(
            [&](const instance::Variable&) -> OptTypeExpression {
                // TODO: var is a TypeModule / Epression or Callable
                return {};
            },
            [&](const instance::Argument&) -> OptTypeExpression { return {}; },
            [&](const instance::Function&) -> OptTypeExpression {
                // TODO: compile time function that returns something useful
                // ++it;
                // auto result = OptNode{};
                // return parseCall(result, fun, it, context);
                return {};
            },
            [&](const instance::Type&) -> OptTypeExpression {
                // this should not occur
                return {};
            },
            [&](const instance::Module& mod) -> OptTypeExpression {
                ++it;
                auto typeNode = mod.locals[View{"type"}];
                if (typeNode) {
                    const auto& type = typeNode->get<instance::Type>();
                    return {TypeInstance{&type}};
                }
                return {};
            });
    }

    template<class Context>
    static auto parseTyped(BlockLineView& it, Context& context) -> OptNamed {
        auto name = it.current().data.visit(
            [&](const block::IdentifierLiteral&) {
                auto result = it.current().range.text;
                ++it;
                return result;
            },
            [](const auto&) { return View{}; });
        auto type = [&]() -> OptTypeExpression {
            if (!it.current().oneOf<block::ColonSeparator>()) return {};
            ++it;
            return parseTypeExpression(it, context);
        }();
        auto value = [&]() -> OptNode {
            if (!it.current().data.visit(
                    [&](const block::OperatorLiteral&) { return it.current().range.text == View{"="}; },
                    [](const auto&) { return false; }))
                return {};
            ++it;
            return parseSingle(it, context);
        }();
        return Named{Name{}, TypedTuple{{Typed{strings::to_string(name), std::move(type), std::move(value)}}}};
    }

    template<class Context>
    using ParseFunc = OptNamed (*)(BlockLineView& it, Context& context);

    static auto getParserForType(const parser::expression::TypeExpression& type) -> instance::Parser {
        using namespace parser::expression;

        return type.visit(
            [&](const Pointer& ptr) {
                return ptr.target->visit(
                    [&](const TypeInstance& inst) { return inst.concrete->parser; },
                    [](const auto&) { return instance::Parser::Expression; });
            },
            [](const auto&) { return instance::Parser::Expression; });
    }

    template<class Context>
    static auto parserForType(const parser::expression::TypeExpression& type) -> ParseFunc<Context> {
        using namespace instance;
        using Parser = instance::Parser;
        switch (getParserForType(type)) {
        case Parser::Expression:
            return [](BlockLineView& it, Context& context) { return parseSingleNamed(it, context); };

        case Parser::SingleToken: return [](BlockLineView& it, Context&) { return parseSingleToken(it); };

        case Parser::IdTypeValue: return [](BlockLineView& it, Context& context) { return parseTyped(it, context); };

        default: return [](BlockLineView&, Context&) { return OptNamed{}; };
        }
    }

    template<class Context>
    static void parseArgumentsWithout(OverloadSet& os, BlockLineView& it, Context& context) {
        os.setupIt(it);
        while (!os.active().empty()) {
            // auto baseIt = it;
            // TODO: optimize for no custom parser case!
            for (auto& o : os.active()) {
                const auto& a = o.arg();
                auto p = parserForType<Context>(a.typed.type);
                auto optNamed = p(o.it, context);
                if (optNamed) {
                    Named& named = optNamed.value();
                    if (!named.name.isEmpty()) {
                        auto optArg = o.function->lookupArgument(named.name);
                        if (optArg) {
                            const instance::Argument& arg = optArg.value();
                            if (canImplicitConvertToType(&named.node, a.typed.type)) {
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
                        if (canImplicitConvertToType(&named.node, a.typed.type)) {
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
