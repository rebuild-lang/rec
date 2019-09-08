#pragma once
#include "CallParser.h"
#include "Context.h"
#include "LineErrorReporter.h"
#include "LineView.h"
#include "TupleLookup.h"
#include "isDirectlyExecutable.h"

#include "parser/Tree.h"

#include "instance/Entry.h"
#include "instance/Function.h"
#include "instance/Module.h"
#include "instance/Type.h"

#include <type_traits>
#include <utility>

namespace parser {

using instance::Function;
using instance::FunctionView;
using strings::CompareView;
using InputBlockLiteral = nesting::BlockLiteral;

struct Parser {
    template<class Context>
    static auto parse(const InputBlockLiteral& blockLiteral, Context context) -> Block {
        auto api = ContextApi<Context>{std::move(context)};
        auto block = Block{};
        for (const auto& line : blockLiteral.value.lines) {
            if (!blockLiteral.isTainted && line.hasErrors()) reportLineErrors(line, api);
            auto it = BlockLineView(&line);
            if (it) {
                auto expr = parseTuple(it, api);
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
    static auto parseTuple(BlockLineView& it, ContextApi<Context>& context) -> NameTypeValueTuple {
        auto tuple = NameTypeValueTuple{};
        auto subContext = context.withTupleLookup(&tuple);
        if (!it) return tuple;
        auto withBrackets = it.current().holds<nesting::BracketOpen>();
        if (withBrackets) ++it;
        parseTupleInto(tuple, it, subContext);
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
    static void parseTupleInto(NameTypeValueTuple& tuple, BlockLineView& it, Context& context) {
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
        return t.visit(
            [](const nesting::OperatorLiteral& o) { return o.input.isContentEqual(View{"="}); }, //
            [](const auto&) { return false; });
    }

    static bool isColon(const BlockToken& t) { return t.holds<nesting::ColonSeparator>(); }

    template<class Context>
    static auto parseSingleTyped(BlockLineView& it, Context& context) -> OptNameTypeValue {
        auto parseValueInto = [&](NameTypeValue& typed) { typed.value = parseSingle(it, context); };
        return parseSingleTypedCallback(it, context, parseValueInto);
    }

    template<class Context, class Callback>
    static auto parseSingleTypedCallback(BlockLineView& it, Context& context, Callback&& callback) -> OptNameTypeValue {
        auto result = NameTypeValue{};
        auto extractName = [&] {
            result.name = to_string(it.current().get<nesting::IdentifierLiteral>().input);
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
    static auto makeTokenValue(BlockLineView&, const Token& token, ContextApi<Context>& context) -> Value {
        auto type = context.intrinsicType(meta::Type<ValueType>{});
        auto value = ValueType{token};
        return {std::move(value), {TypeInstance{type}}};
    }

    template<class Context>
    static auto parseStep(OptNode& result, BlockLineView& it, ContextApi<Context>& context) -> ParseOptions {
        auto parseId = [&](const auto& id) {
            using Id = std::remove_const_t<std::remove_reference_t<decltype(id)>>;
            if (auto range = lookupModule(id.input, result); !range.empty()) {
                result = {};
                return parseInstance(result, range, it, context);
            }
            if (auto opRef = context.lookupTuple(id.input); opRef) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{NameTypeValueReference{opRef.value()}};
                ++it;
                return ParseOptions::continue_single;
            }
            if (auto range = context.lookup(id.input); !range.empty()) {
                return parseInstance(result, range, it, context);
            }
            // symbol not found
            if (result) return ParseOptions::finish_single;
            result = OptNode{makeTokenValue<Id>(it, id, context)};
            ++it;
            return ParseOptions::continue_single;
        };
        auto parseLiteral = [&](const auto& lit) {
            using Lit = std::remove_const_t<std::remove_reference_t<decltype(lit)>>;
            if (result) return ParseOptions::finish_single;
            result = OptNode{makeTokenValue<Lit>(it, lit, context)};
            ++it;
            return ParseOptions::continue_single;
        };
        return it.current().visit(
            [](const nesting::CommaSeparator&) { return ParseOptions::finish_single; },
            [](const nesting::BracketClose&) { return ParseOptions::finish_single; },
            [&](const nesting::BracketOpen&) {
                if (result) return ParseOptions::finish_single;
                auto tuple = parseTuple(it, context);
                result = Node{std::move(tuple)};
                return ParseOptions::continue_single;
            },
            [&](const nesting::IdentifierLiteral& id) { return parseId(id); },
            [&](const nesting::OperatorLiteral& op) { return parseId(op); },
            [&](const nesting::StringLiteral& s) { return parseLiteral(s); },
            [&](const nesting::NumberLiteral& n) { return parseLiteral(n); },
            [&](const nesting::BlockLiteral& b) { return parseLiteral(b); },
            [](const auto&) { return ParseOptions::finish_single; });
    }

    static auto lookupModule(const strings::View& id, const OptNode& result) -> instance::ConstEntryRange {
        return result.map([&](const Node& n) -> instance::ConstEntryRange {
            return n.visit(
                [&](const ModuleReference& ref) { return ref.module->locals[id]; },
                // [&](const VariableReference& ref) {},
                // [&](const ParameterReference& ref) {},
                // [&](const NameTypeValueReference& ref) {},
                [](const auto&) { return instance::ConstEntryRange{}; });
        });
    }

    template<class Context>
    static auto parseInstance( //
        OptNode& result,
        const instance::ConstEntryRange& range,
        BlockLineView& it,
        Context& context) -> ParseOptions {

        return range.frontValue().visit(
            [&](const instance::Variable& var) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{VariableReference{&var}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::Parameter& arg) {
                if (result) return ParseOptions::finish_single;
                result = OptNode{ParameterReference{&arg}};
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
        // TODO(arBmind): add overloads
    }

    static bool canImplicitConvertToType(NodeView node, const parser::TypeExpression& type) {
        // TODO(arBmind):
        // I guess we need a scope here
        (void)node;
        (void)type;
        return true;
    }

    static void assignLeftArguments(CallOverloads& co, const OptNode& left) {
        auto leftView = left //
            ? left.value().holds<NameTypeValueTuple>() //
                ? ViewNameTypeValueTuple{left.value().get<NameTypeValueTuple>()}
                : ViewNameTypeValueTuple{left.value()}
            : ViewNameTypeValueTuple{};
        for (auto& coi : co.items) {
            auto o = 0u, t = 0u;
            auto la = coi.function->leftParameters();
            for (const ViewNameTypeValue& typed : leftView.tuple) {
                if (!typed.value) {
                    // pass type?
                }
                else if (typed.name) {
                    auto optParam = coi.function->lookupParameter(typed.name.value());
                    if (optParam) {
                        instance::ParameterView param = optParam.value();
                        if (param->side == instance::ParameterSide::left //
                            && canImplicitConvertToType(typed.value.value(), param->typed.type)) {
                            t++;
                            continue;
                        }
                        // side does not match
                        // type does not match
                    }
                    // name not found
                }
                else if (o < la.size()) {
                    const auto* param = la[o];
                    if (param->side == instance::ParameterSide::left //
                        && canImplicitConvertToType(typed.value.value(), param->typed.type)) {
                        o++;
                        continue;
                    }
                    // side does not match
                    // type does not match
                }
                // index out of range
                coi.active = false;
                return;
            }
            if (o + t == la.size()) return;
            // not right count
            coi.active = false;
        }
    }

    template<class Context>
    struct Wrap {
        template<class Type>
        auto intrinsicType(meta::Type<Type>) -> instance::TypeView {
            return context->intrinsicType(meta::Type<Type>{});
        }
        auto reportDiagnostic(diagnostic::Diagnostic diagnostic) -> void {
            return context->reportDiagnostic(std::move(diagnostic));
        }
        auto parserForType(const TypeExpression& type) {
            return [this, parser = Parser::parserForType<Context>(type)](BlockLineView& blv) { //
                return parser(blv, *context);
            };
        }
        template<class Callback>
        auto parseTypedWithCallback(BlockLineView& it, Callback&& cb) -> OptNameTypeValue {
            return Parser::parseSingleTypedCallback(it, *context, cb);
        }
        Context* context;
    };

    template<class Context>
    static auto parseCall( //
        OptNode& left,
        const Function& fun,
        BlockLineView& it,
        Context& context) -> ParseOptions { //

        auto co = CallOverloads{};
        co.items.emplace_back(&fun);
        assignLeftArguments(co, left);

        CallParser::parse(co, it, Wrap<Context>{&context});
        if (co.countComplete() == 1) {
            auto& ci = co.items.front();
            left = buildCallNode(Call{ci.function, ci.args}, context);
            return ci.hasBlocks ? ParseOptions::finish_single : ParseOptions::continue_single;
        }

        if (left) return ParseOptions::finish_single;
        // left = OptNode{FunctionReference{fun}};
        return ParseOptions::continue_single;
    }

    template<class Context>
    static auto buildCallNode(Call&& call, ContextApi<Context>& context) -> OptNode {
        if (isDirectlyExecutable(call)) {
            return context.runCall(call);
        }
        return Node{std::move(call)};
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
    static auto parseTypeExpression(BlockLineView& it, ContextApi<Context>& context) -> OptTypeExpression {
        return it.current().visit(
            [&](const nesting::IdentifierLiteral& id) -> OptTypeExpression {
                auto name = id.input;
                auto range = context.lookup(name);
                if (range.single()) return parseTypeInstance(range.frontValue(), it, context);
                return {};
            },
            [](const auto&) -> OptTypeExpression { // error
                return {};
            });
    }

    template<class Context>
    static auto parseTypeInstance(const instance::Entry& instance, BlockLineView& it, Context& context)
        -> OptTypeExpression {
        return instance.visit(
            [&](const instance::Variable&) -> OptTypeExpression {
                // TODO(arBmind): var is a TypeModule / Expression or Callable
                return {};
            },
            [&](const instance::Parameter&) -> OptTypeExpression { return {}; },
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
                    auto subName = it.current().get<nesting::IdentifierLiteral>().input;
                    auto subRange = mod.locals[subName];
                    if (subRange.single()) return parseTypeInstance(subRange.frontValue(), it, context);
                }
                auto typeRange = mod.locals[View{"type"}];
                if (typeRange.single()) {
                    const auto& type = typeRange.frontValue().get<instance::Type>();
                    return {TypeInstance{&type}};
                }
                // error
                return {};
            });
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
        case Parser::Expression:
            return [](BlockLineView& it, Context& context) {
                return parseSingle(it, context); //
            };
        case Parser::SingleToken:
            return [](BlockLineView& it, Context& context) { //
                return parseSingleToken(it, context);
            };
        case Parser::IdTypeValue:
            return [](BlockLineView& it, Context& context) -> OptNode {
                auto optTyped = parseSingleTyped(it, context);
                if (optTyped) {
                    auto type = context.intrinsicType(meta::Type<Typed>{});
                    auto value = NameTypeValue{optTyped.value()};
                    return {Value{std::move(value), {TypeInstance{type}}}};
                }
                // return Node{TypedTuple{{optTyped.value()}}}; // TODO(arBmind): store as value
                return {};
            };

        default: return [](BlockLineView&, Context&) { return OptNode{}; };
        }
    }
};

} // namespace parser
