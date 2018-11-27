#pragma once
#include "LineView.h"

#include "parser/Tree.h"

#include "instance/Function.h"
#include "instance/Module.h"
#include "instance/Node.h"
#include "instance/Type.h"

#include <type_traits>
#include <utility>

namespace parser {

using Function = instance::Function;
using FunctionView = instance::FunctionView;
using InputBlockLiteral = nesting::BlockLiteral;

template<class Lookup, class RunCall, class IntrinsicType>
struct Context {
    Lookup lookup;
    RunCall runCall;
    IntrinsicType intrinsicType;
};

// template deduction guide
template<class Lookup, class RunCall, class IntrinsicType>
Context(Lookup&&, RunCall&&, IntrinsicType &&)->Context<Lookup, RunCall, IntrinsicType>;

template<class Context>
constexpr void checkContext() noexcept {
    static_assert(
        std::is_same_v<instance::OptConstNodeView, std::invoke_result_t<decltype(Context::lookup), strings::View>>,
        "no lookup");
    static_assert(
        std::is_same_v<OptNode, std::invoke_result_t<decltype(Context::runCall), Call>>, //
        "no runCall");
    static_assert(
        std::is_same_v<
            instance::TypeView,
            std::invoke_result_t<decltype(Context::intrinsicType), meta::Type<StringLiteral>>>,
        "no intrinsicType");
}

struct Parser {
    template<class Context>
    static auto parse(const InputBlockLiteral& blockLiteral, Context context) -> Block {
        checkContext<Context>();
        auto block = Block{};
        for (const auto& line : blockLiteral.value.lines) {
            auto it = BlockLineView(&line);
            if (it) {
                auto expr = parseTuple(it, context);
                if (!expr.tuple.empty()) {
                    if (1 == expr.tuple.size() && expr.tuple.front().onlyValue()) {
                        // no reason to keep the tuple around, unwrap it
                        block.nodes.emplace_back(std::move(expr).tuple.front().value.value());
                    }
                    else {
                        block.nodes.emplace_back(std::move(expr));
                    }
                }
                if (it) {
                    // TODO(arBmind): report remaining tokens on line
                    // handling: ignore / maybe try to parse?
                }
            }
        }
        return block;
    }

private:
    template<class Context>
    static auto parseTuple(BlockLineView& it, Context& context) -> TypedTuple {
        auto tuple = TypedTuple{};
        if (!it) return tuple;
        auto withBrackets = it.current().holds<nesting::BracketOpen>();
        if (withBrackets) ++it;
        parseTupleInto(tuple, it, context);
        if (withBrackets) {
            if (!it) {
                // error: missing closing bracket
            }
            else if (!it.current().holds<nesting::BracketClose>()) {
                // error: missing closing bracket
            }
            else {
                ++it;
            }
        }
        return tuple;
    }

    template<class Context>
    static void parseTupleInto(TypedTuple& tuple, BlockLineView& it, Context& context) {
        while (it) {
            auto opt = parseSingleTyped(it, context);
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
        if (it.current().holds<nesting::CommaSeparator>()) {
            ++it; // skip optional comma
            if (!it) return ParseOptions::finish_single;
        }
        if (it.current().holds<nesting::BracketClose>()) return ParseOptions::finish_single;
        return ParseOptions::continue_single;
    }

    static bool isAssignment(const BlockToken& t) {
        /* return t.visit(
            [](const nesting::OperatorLiteral& o) { return o.range.view.isContentEqual(View{"="}); }, //
            [](const auto&) { return false; });
            */
        return false;
    }

    static bool isColon(const BlockToken& t) { return t.holds<nesting::ColonSeparator>(); }

    template<class Context>
    static auto parseSingleTyped(BlockLineView& it, Context& context) -> OptTyped {
        auto parseValueInto = [&](Typed& typed) { typed.value = parseSingle(it, context); };
        return parseSingleTypedCallback(it, context, parseValueInto);
    }

    template<class Context, class Callback>
    static auto parseSingleTypedCallback(BlockLineView& it, Context& context, Callback&& callback) -> OptTyped {
        auto result = Typed{};
        auto extractName = [&] {
            // result.name = to_string(it.current().get<nesting::IdentifierLiteral>().range.view);
            ++it; // skip name
        };
        auto parseType = [&] {
            ++it; // skip colon
            result.type = parseTypeExpression(it, context);
        };
        auto parseValue = [&] {
            ++it; // skip assign
            callback(result);
        };
        auto parseAssignValue = [&]() {
            if (it && isAssignment(it.current())) parseValue();
        };
        if (!it) return result;

        if (it.current().holds<nesting::IdentifierLiteral>() && it.hasNext()) {
            auto next = it.next();
            if (isColon(next)) {
                // name :type
                extractName();
                parseType();
                parseAssignValue();
                return result;
            }
            if (isAssignment(next)) {
                // name =value
                extractName();
                parseValue();
                return result;
            }
        }
        if (isColon(it.current())) {
            // :typed
            parseType();
            parseAssignValue();
            return result;
        }
        // value
        callback(result);
        if (!result.value) return {};
        return result;
    }

    template<class Context>
    static auto parseAssignmentNode(BlockLineView& it, Context& context) -> OptNode {
        if (it && isAssignment(it.current())) {
            // = value
            ++it;
            return parseSingle(it, context);
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

    template<class ValueType, class Context, class Token>
    static auto makeTokenValue(BlockLineView& it, const Token& token, Context& context) -> Value {
        auto type = context.intrinsicType(meta::Type<ValueType>{});
        auto value = ValueType{token};
        return {std::move(value), {TypeInstance{type}}};
    }

    template<class Context>
    static auto parseStep(OptNode& result, BlockLineView& it, Context& context) -> ParseOptions {
        return it.current().visit(
            [](const nesting::CommaSeparator&) { return ParseOptions::finish_single; },
            [](const nesting::BracketClose&) { return ParseOptions::finish_single; },
            [&](const nesting::BracketOpen&) {
                if (result) return ParseOptions::finish_single;
                auto tuple = parseTuple(it, context);
                result = Node{std::move(tuple)};
                return ParseOptions::continue_single;
            },
            [&](const nesting::IdentifierLiteral& id) {
                /*auto optNode = lookupIdentifier(id.range.view, result, context);
                if (!optNode) {
                    if (result) return ParseOptions::finish_single;

                    result = OptNode{makeTokenValue<IdentifierLiteral>(it, id, context)};
                    ++it;
                    return ParseOptions::continue_single;
                }
                return parseInstance(result, *optNode.value(), it, context);
                */
                return ParseOptions::continue_single;
            },
            [&](const nesting::StringLiteral& s) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{makeTokenValue<StringLiteral>(it, s, context)};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const nesting::NumberLiteral& n) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{makeTokenValue<NumberLiteral>(it, n, context)};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const nesting::BlockLiteral& b) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{makeTokenValue<BlockLiteral>(it, b, context)};
                ++it;
                return ParseOptions::continue_single;
            },
            [](const auto&) { return ParseOptions::finish_single; });
    }

    template<class Context>
    static auto lookupIdentifier(const strings::View& id, OptNode& result, Context& context)
        -> instance::OptConstNodeView {
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
        // TODO(arBmind): add modules
        // TODO(arBmind): add overloads
    }

    static bool canImplicitConvertToType(NodeView node, const parser::TypeExpression& type) {
        // TODO(arBmind):
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
            bool hasBlocks{false};
            FunctionView function{};
            BlockLineView it{};
            ArgumentAssignments rightArgs{};
            size_t nextArg{};

            Overload() = default;
            explicit Overload(const Function& function)
                : function(&function)
                , active(!function.arguments.empty())
                , complete(function.arguments.empty()) {}

            void retireLeft(const TypedViewTuple& left) {
                auto o = 0u, t = 0u;
                auto la = function->leftArguments();
                for (const TypedView& typed : left.tuple) {
                    if (!typed.value) {
                        // pass type?
                    }
                    else if (typed.name) {
                        auto optArg = function->lookupArgument(typed.name.value());
                        if (optArg) {
                            instance::ArgumentView arg = optArg.value();
                            if (arg->side == instance::ArgumentSide::left //
                                && canImplicitConvertToType(typed.value.value(), arg->typed.type)) {
                                t++;
                                continue;
                            }
                            // side does not match
                            // type does not match
                        }
                        // name not found
                    }
                    else if (o < la.size()) {
                        const auto* arg = la[o];
                        if (arg->side == instance::ArgumentSide::left //
                            && canImplicitConvertToType(typed.value.value(), arg->typed.type)) {
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

            auto arg() const -> instance::ArgumentView { return function->rightArguments()[nextArg]; }
        };
        using Overloads = std::vector<Overload>;

        explicit OverloadSet(const Function& fun) {
            vec.emplace_back(fun);
            activeCount = 1;
        }
        // TODO(arBmind): allow multiple overloads

        void retireLeft(const OptNode& left) {
            auto leftView = left ? left.value().holds<TypedTuple>() ? TypedViewTuple{left.value().get<TypedTuple>()}
                                                                    : TypedViewTuple{left.value()}
                                 : TypedViewTuple{};
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
        int64_t activeCount{};
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
        }
        auto completed = os.finish();
        if (completed.size() == 1) {
            auto& o = completed.front();
            it = o.it;
            auto inv = [&] {
                auto r = Call{};
                r.function = o.function;
                // TODO(arBmind): assign left arguments
                r.arguments = o.rightArgs;
                return r;
            };
            left = buildCallNode(inv(), context);
            return o.hasBlocks ? ParseOptions::finish_single : ParseOptions::continue_single;
        }

        if (left) return ParseOptions::finish_single;
        // left = OptNode{FunctionReference{fun}};
        return ParseOptions::continue_single;
    }

    static bool isDirectlyExecutable(const TypeExpression& expr) {
        return expr.visit(
            [](const Auto&) { return false; }, //
            [](const auto&) { return true; });
    }

    static bool isDirectlyExecutable(const Typed& typed) {
        if (typed.value && !isDirectlyExecutable(typed.value.value())) return false;
        if (typed.type && !isDirectlyExecutable(typed.type.value())) return false;
        return true;
    }

    static bool isDirectlyExecutable(const Node& node) {
        return node.visit(
            [](const Block&) { return false; },
            [](const Call& call) { return isDirectlyExecutable(call); },
            [](const IntrinsicCall&) { return false; },
            [](const ArgumentReference&) { return false; },
            [](const VariableReference&) { return false; },
            [](const VariableInit&) { return false; },
            [](const ModuleReference&) { return false; },
            [](const TypedTuple& tuple) {
                for (auto& typed : tuple.tuple)
                    if (!isDirectlyExecutable(typed)) return false;
                return true;
            },
            [](const Value&) { return true; });
    }

    static bool isDirectlyExecutable(const Call& call) {
        if (call.function->flags.none(instance::FunctionFlag::compile_time)) return false;
        for (auto& arg : call.arguments) {
            for (auto& node : arg.values) {
                if (!isDirectlyExecutable(node)) return false;
            }
        }
        return true;
    }

    template<class Context>
    static auto buildCallNode(Call&& call, Context& context) -> OptNode {
        if (isDirectlyExecutable(call)) {
            return context.runCall(call);
        }
        return OptNode{{call}};
    }

    template<class Context>
    static void parseArguments(OverloadSet& os, BlockLineView& it, Context& context) {
        auto withBrackets = it.current().holds<nesting::BracketOpen>();
        if (withBrackets) ++it;
        parseArgumentsWithout(os, it, context);
        if (withBrackets) {
            if (!it) {
                // error: missing closing bracket
            }
            else if (!it.current().holds<nesting::BracketClose>()) {
                // error: missing closing bracket
            }
            else {
                ++it;
            }
        }
    }

    template<class Token, class Context>
    struct TokenVisitor {
        BlockLineView& it;
        Context& context;
        using BlockToken = Token;

        auto operator()(const BlockToken& t) -> OptNode {
            auto result = OptNode{makeTokenValue<Token>(it, t, context)};
            ++it;
            return result;
        }
    };
    template<class Context, class... T>
    constexpr static auto buildTokenVisitors(BlockLineView& it, Context& context, meta::TypeList<T...> = {}) {
        return meta::Overloaded{TokenVisitor<T, Context>{it, context}...};
    }

    template<class Context>
    static auto parseSingleToken(BlockLineView& it, Context& context) -> OptNode {
        return it.current().visit(
            buildTokenVisitors<Context, BlockLiteral, StringLiteral, NumberLiteral, IdentifierLiteral, OperatorLiteral>(
                it, context),
            [](const auto&) { return OptNode{}; });
    }

    template<class Context>
    static auto parseTypeExpression(BlockLineView& it, Context& context) -> OptTypeExpression {
        return it.current().visit(
            [&](const nesting::IdentifierLiteral& id) -> OptTypeExpression {
                /* auto name = id.range.view;
                auto node = context.lookup(name);
                if (node) return parseTypeInstance(*node.value(), it, context);
                */
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
                // TODO(arBmind): var is a TypeModule / Expression or Callable
                return {};
            },
            [&](const instance::Argument&) -> OptTypeExpression { return {}; },
            [&](const instance::Function&) -> OptTypeExpression {
                // TODO(arBmind): compile time function that returns something useful
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
                if (it && it.current().holds<nesting::IdentifierLiteral>()) {
                    auto subName = it.current().get<nesting::IdentifierLiteral>().range.view;
                    auto subNode = mod.locals[subName];
                    if (subNode) return parseTypeInstance(*subNode.value(), it, context);
                }
                auto typeNode = mod.locals[View{"type"}];
                if (typeNode) {
                    const auto& type = typeNode.value()->get<instance::Type>();
                    return {TypeInstance{&type}};
                }
                // error
                return {};
            });
    }

    template<class Context>
    static auto parseTyped(BlockLineView& it, Context& context) -> OptTyped {
        auto name = it.current().visit(
            [&](const nesting::IdentifierLiteral& id) {
                auto result = id.range.view;
                ++it;
                return result;
            },
            [](const auto&) { return View{}; });
        auto type = [&]() -> OptTypeExpression {
            if (!it.current().holds<nesting::ColonSeparator>()) return {};
            ++it;
            return parseTypeExpression(it, context);
        }();
        auto value = [&]() -> OptNode {
            if (!it.current().visit(
                    [&](const nesting::OperatorLiteral& op) { return op.range.view == View{"="}; },
                    [](const auto&) { return false; }))
                return {};
            ++it;
            return parseSingle(it, context);
        }();
        return Typed{strings::to_string(name), std::move(type), std::move(value)};
    }

    template<class Context>
    using ParseFunc = auto (*)(BlockLineView& it, Context& context) -> OptNode;

    static auto getParserForType(const parser::TypeExpression& type) -> instance::Parser {
        using parser::Pointer;
        using parser::TypeInstance;

        return type.visit(
            [&](const Pointer& ptr) {
                return ptr.target->visit(
                    [&](const TypeInstance& inst) { return inst.concrete->parser; },
                    [](const auto&) { return instance::Parser::Expression; });
            },
            [](const auto&) { return instance::Parser::Expression; });
    }

    template<class Context>
    static auto parserForType(const parser::TypeExpression& type) -> ParseFunc<Context> {
        using namespace instance;
        using Parser = instance::Parser;
        switch (getParserForType(type)) {
        case Parser::Expression: return [](BlockLineView& it, Context& context) { return parseSingle(it, context); };

        case Parser::SingleToken:
            return [](BlockLineView& it, Context& context) { return parseSingleToken(it, context); };

        case Parser::IdTypeValue:
            return [](BlockLineView& it, Context& context) -> OptNode {
                auto optTyped = parseSingleTyped(it, context);
                if (optTyped) {
                    auto type = context.intrinsicType(meta::Type<Typed>{});
                    auto value = Typed{optTyped.value()};
                    return {Value{std::move(value), {TypeInstance{type}}}};
                }
                // return Node{TypedTuple{{optTyped.value()}}}; // TODO(arBmind): store as value
                return {};
            };

        default: return [](BlockLineView&, Context&) { return OptNode{}; };
        }
    }

    template<class Context>
    static bool isTyped(const TypeExpression& t, Context& context) {
        return t.visit(
            [&](const TypeInstance& ti) { return ti.concrete == context.intrinsicType(meta::Type<parser::Typed>{}); },
            [](const auto&) { return false; });
    }
    template<class Context>
    static bool isBlockLiteral(const TypeExpression& t, Context& context) {
        return t.visit(
            [&](const TypeInstance& ti) {
                return ti.concrete == context.intrinsicType(meta::Type<parser::BlockLiteral>{});
            },
            [](const auto&) { return false; });
    }

    template<class Context>
    static void parseArgumentsWithout(OverloadSet& os, BlockLineView& it, Context& context) {
        os.setupIt(it);
        while (!os.active().empty()) {
            // auto baseIt = it;
            // TODO(arBmind): optimize for no custom parser case!
            for (auto& o : os.active()) {
                auto* posArg = o.arg();
                auto parseValueArgument = [&](Typed& typed) {
                    if (typed.name && !typed.type) {
                        auto optNamedArg = o.function->lookupArgument(typed.name.value());
                        if (optNamedArg) {
                            instance::ArgumentView namedArg = optNamedArg.value();
                            auto p = parserForType<Context>(namedArg->typed.type);
                            typed.value = p(o.it, context);
                            return;
                        }
                        else {
                            // error: name not found / unless a == Typed{}
                        }
                    }
                    auto p = parserForType<Context>(posArg->typed.type);
                    typed.value = p(o.it, context);
                };

                auto optTyped = parseSingleTypedCallback(o.it, context, parseValueArgument);
                // auto p = parserForType<Context>(a.typed.type);
                // auto optTyped = p(o.it, context);
                if (optTyped) {
                    Typed& typed = optTyped.value();
                    do {
                        if (typed.type || !typed.value) {
                            if (isTyped(posArg->typed.type, context)) {
                                auto as = ArgumentAssignment{};
                                as.argument = posArg;
                                auto type = context.intrinsicType(meta::Type<Typed>{});
                                auto value = Typed{typed};
                                as.values = {Value{std::move(value), {TypeInstance{type}}}};
                                o.rightArgs.push_back(std::move(as));
                                o.nextArg++;
                                break;
                            }
                            // unexpected type / no value
                            break;
                        }
                        auto isNodeBlockLiteral = [&](OptNode& node) {
                            if (!node) return false;
                            if (!node.value().holds<Value>()) return false;
                            auto& value = node.value().get<Value>();
                            return isBlockLiteral(value.type(), context);
                        };
                        if (isNodeBlockLiteral(typed.value)) o.hasBlocks = true;

                        if (typed.name) {
                            auto optNamedArg = o.function->lookupArgument(typed.name.value());
                            if (optNamedArg) {
                                instance::ArgumentView namedArg = optNamedArg.value();
                                if (canImplicitConvertToType(&typed.value.value(), namedArg->typed.type)) {
                                    auto as = ArgumentAssignment{};
                                    as.argument = namedArg;
                                    as.values = {std::move(optTyped).value().value.value()};
                                    o.rightArgs.push_back(std::move(as));
                                    // TODO(arBmind): add to call completion
                                    break;
                                }
                                // type does not match
                                break;
                            }
                            else {
                                // name not found
                                break;
                            }
                        }
                        if (canImplicitConvertToType(&typed.value.value(), posArg->typed.type)) {
                            auto as = ArgumentAssignment{};
                            as.argument = posArg;
                            as.values = {std::move(optTyped).value().value.value()};
                            o.rightArgs.push_back(std::move(as));
                            o.nextArg++;
                            break;
                        }
                        // type does not match
                    } while (false);
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

} // namespace parser
