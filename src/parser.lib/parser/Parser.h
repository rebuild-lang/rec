#pragma once
#include "CallParser.h"
#include "Context.h"
#include "LineErrorReporter.h"
#include "LineView.h"
#include "TupleLookup.h"
#include "isDirectlyExecutable.h"

#include "parser/Expression.h"

#include "instance/Entry.h"
#include "instance/Function.h"
#include "instance/Module.h"
#include "instance/Type.h"

#include <type_traits>
#include <utility>

#include <cassert>

namespace parser {

using instance::Function;
using instance::FunctionView;
using strings::CompareView;
using InputBlockLiteral = nesting::BlockLiteral;

/// Pointers to name, type and value (used internally)
struct ViewNameTypeValue {
    using This = ViewNameTypeValue;
    OptView name{};
    OptTypeExprView type{};
    OptValueExprView value{};

    ViewNameTypeValue() = default;
    ViewNameTypeValue(const NameTypeValue& ntv) noexcept
        : name(ntv.name.map([](const auto& n) -> View { return n; }))
        , type(ntv.type.map([](const auto& t) -> TypeExprView { return &t; }))
        , value(ntv.value.map([](const auto& v) -> ValueExprView { return &v; })) {}
    explicit ViewNameTypeValue(const ValueExpr& node) noexcept
        : value(&node) {}
};
using VecOfViewNameTypeValue = std::vector<ViewNameTypeValue>;

static_assert(meta::has_move_assignment<ViewNameTypeValue>);

struct ViewNameTypeValueTuple {
    using This = ViewNameTypeValueTuple;
    VecOfViewNameTypeValue tuple{};

    ViewNameTypeValueTuple() = default;
    ViewNameTypeValueTuple(const NameTypeValueTuple& ntvTuple) noexcept
        : tuple(ntvTuple.tuple.begin(), ntvTuple.tuple.end()) {}
    explicit ViewNameTypeValueTuple(const ValueExpr& node) noexcept
        : tuple({ViewNameTypeValue{node}}) {}
};
static_assert(meta::has_move_assignment<ViewNameTypeValueTuple>);

struct Parser {
    template<class C>
    [[nodiscard]] static auto parseBlock(const InputBlockLiteral& blockLiteral, C context) -> Block {
        static_assert(is_context<C>);
        auto block = Block{};
        for (const auto& line : blockLiteral.value.lines) {
            if (!blockLiteral.isTainted && line.hasErrors()) reportLineErrors(line, context);
            auto it = BlockLineView(&line);
            if (it) {
                auto expr = parseNameTypeValueTuple(it, context);
                if (!expr.tuple.empty()) {
                    if (1 == expr.tuple.size() && expr.tuple.front().onlyValue()) {
                        // no reason to keep the tuple around, unwrap it
                        block.expressions.emplace_back(std::move(expr).tuple.front().value.value().visit(
                            [](auto&& v) -> BlockExpr { return std::move(v); }));
                    }
                    else {
                        block.expressions.emplace_back(std::move(expr));
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
    template<class C>
    [[nodiscard]] static auto parseNameTypeValueTuple(BlockLineView& it, C& context) -> NameTypeValueTuple {
        auto tuple = NameTypeValueTuple{};
        using BaseContext = typename C::BaseContext;
        auto subContext = ContextWithTupleLookup<BaseContext>(context, &tuple);
        if (!it) return tuple;
        auto withBrackets = it.current().holds<nesting::BracketOpen>();
        if (withBrackets) ++it;
        while (it) {
            auto opt = parseNameTypeValue(it, subContext);
            if (opt) {
                tuple.tuple.push_back(std::move(opt).value());
            }
            auto r = parseOptionalComma(it);
            if (r == ParseOptions::finish_single) break;
        }
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

    enum class ParseOptions {
        continue_single,
        finish_single,
    };

    [[nodiscard]] static auto parseOptionalComma(BlockLineView& it) -> ParseOptions {
        if (!it) return ParseOptions::finish_single;
        if (it.current().holds<nesting::CommaSeparator>()) {
            ++it; // skip optional comma
            if (!it) return ParseOptions::finish_single;
        }
        if (it.current().holds<nesting::BracketClose>()) return ParseOptions::finish_single;
        return ParseOptions::continue_single;
    }

    [[nodiscard]] static auto isAssignment(const BlockToken& t) -> bool {
        return t.visit(
            [](const nesting::IdentifierLiteral& o) { return o.input.isContentEqual(View{"="}); }, //
            [](const auto&) { return false; });
    }

    [[nodiscard]] static auto isColon(const BlockToken& t) -> bool { return t.holds<nesting::ColonSeparator>(); }

    template<class Context>
    [[nodiscard]] static auto parseNameTypeValue(BlockLineView& it, Context& context) -> OptNameTypeValue {
        auto parseValueInto = [&](NameTypeValue& ntv) { ntv.value = parseValueExpr(it, context); };
        return parseNameTypeValueCallback(it, context, parseValueInto);
    }

    template<class Context, class Callback>
    [[nodiscard]] static auto parseNameTypeValueCallback(
        BlockLineView& it, Context& context, Callback&& parseValueCallback) -> OptNameTypeValue {
        auto nameTypeValue = NameTypeValue{};
        auto extractName = [&] {
            nameTypeValue.name = to_string(it.current().get<nesting::IdentifierLiteral>().input);
            ++it; // skip name
        };
        auto parseType = [&] {
            ++it; // skip colon
            nameTypeValue.type = parseTypeExpr(it, context);
        };
        auto parseValue = [&] {
            ++it; // skip assign
            parseValueCallback(nameTypeValue);
        };
        auto parseAssignValue = [&]() {
            if (it && isAssignment(it.current())) parseValue();
        };
        if (!it) return nameTypeValue;

        if (it.current().holds<nesting::IdentifierLiteral>() && it.hasNext()) {
            auto next = it.next();
            if (isColon(next)) {
                // name :type
                extractName();
                parseType();
                parseAssignValue();
                return nameTypeValue;
            }
            if (isAssignment(next)) {
                // name =value
                extractName();
                parseValue();
                return nameTypeValue;
            }
        }
        if (isColon(it.current())) {
            // :typed
            parseType();
            parseAssignValue();
            return nameTypeValue;
        }
        // just value (no assign)
        parseValueCallback(nameTypeValue);
        if (!nameTypeValue.value) return {};
        return nameTypeValue;
    }

    template<class ContextBase>
    [[nodiscard]] static auto parseValueExpr(BlockLineView& it, ContextWithTupleLookup<ContextBase>& context)
        -> OptValueExpr {
        auto result = OptValueExpr{};
        auto parseTuple = [&]() -> ParseOptions {
            if (result) return ParseOptions::finish_single;
            auto tuple = parseNameTypeValueTuple(it, context);
            result = ValueExpr{std::move(tuple)};
            return ParseOptions::continue_single;
        };
        auto parseId = [&](const auto& id) -> ParseOptions {
            using Id = std::remove_const_t<std::remove_reference_t<decltype(id)>>;
            if (auto range = lookupModule(id.input, result); !range.empty()) {
                result = {};
                return parseInstance(result, range, it, context);
            }
            if (auto opRef = context.lookupTuple(id.input); opRef) {
                if (result) return ParseOptions::finish_single;
                result = OptValueExpr{NameTypeValueReference{opRef.value()}};
                ++it;
                return ParseOptions::continue_single;
            }
            if (auto range = context.lookup(id.input); !range.empty()) {
                return parseInstance(result, range, it, context);
            }
            // symbol not found
            if (result) return ParseOptions::finish_single;
            result = OptValueExpr{makeTokenValue<Id>(it, id, context)};
            ++it;
            return ParseOptions::continue_single;
        };
        auto parseLiteral = [&](const auto& lit) -> ParseOptions {
            using Lit = std::remove_const_t<std::remove_reference_t<decltype(lit)>>;
            if (result) return ParseOptions::finish_single;
            result = OptValueExpr{makeTokenValue<Lit>(it, lit, context)};
            ++it;
            return ParseOptions::continue_single;
        };

        while (it) {
            auto opt = it.current().visit(
                [](const nesting::CommaSeparator&) { return ParseOptions::finish_single; },
                [](const nesting::BracketClose&) { return ParseOptions::finish_single; },
                [&](const nesting::BracketOpen&) { return parseTuple(); },
                [&](const nesting::IdentifierLiteral& id) { return parseId(id); },
                [&](const nesting::StringLiteral& s) { return parseLiteral(s); },
                [&](const nesting::NumberLiteral& n) { return parseLiteral(n); },
                [&](const nesting::BlockLiteral& b) { return parseLiteral(b); },
                [](const auto&) { return ParseOptions::finish_single; });

            if (opt == ParseOptions::finish_single) break;
        }
        return result;
    }

    template<class Context>
    [[nodiscard]] static auto parseTypeExpr(BlockLineView& it, Context& context) -> OptTypeExpr {
        auto result = OptTypeExpr{};
        auto optValue = parseValueExpr(it, context);
        if (optValue) {
            std::move(optValue).value().visit(
                [&](auto&& val) { result = {std::move(val)}; },
                [&](ModuleReference&& mod) {
                    auto f = mod.module->locals.byName(parser::nameOfType());
                    if (f.single() && f.frontValue().holds<instance::TypePtr>()) {
                        result = {TypeReference{f.frontValue().get<instance::TypePtr>().get()}};
                    }
                    // TODO: extract Type of module…
                },
                [](NameTypeValueTuple&& tuple) {
                    // TODO: one member + value of type 'Type'
                },
                [](Block&&) {
                    // TODO: report error
                },
                [](VariableInit&&) {
                    // TODO: report error
                });
        }
        return result;
    }

    template<class Context>
    [[nodiscard]] static auto parsePartialExpr(BlockLineView& it, Context& context) -> VecOfPartiallyParsed {
        auto result = VecOfPartiallyParsed{};
        auto parseSubExpr = [&]() {
            ++it; // skip signal
            auto subExpr = parseValueExpr(it, context);
            if (subExpr) {
                std::move(subExpr).value().visit(
                    [&](auto&& val) { result.emplace_back(std::move(val)); },
                    [&](VecOfPartiallyParsed&& subPartial) {
                        for (auto&& val : subPartial) {
                            result.emplace_back(std::move(val));
                        }
                    });
            }
        };
        it.current.visit(
            [](const nesting::CommaSeparator&) { return ParseOptions::finish_single; },
            [](const nesting::BracketClose&) { return ParseOptions::finish_single; },
            [&](const nesting::BracketOpen&) {
                if (result) return ParseOptions::finish_single;
                auto tuple = parseNameTypeValueTuple(it, context);
                result = ValueExpr{std::move(tuple)};
                return ParseOptions::continue_single;
            },
            [&](const nesting::IdentifierLiteral& id) { return parseId(id); },
            [&](const nesting::StringLiteral& s) { return parseLiteral(s); },
            [&](const nesting::NumberLiteral& n) { return parseLiteral(n); },
            [&](const nesting::BlockLiteral& b) { return parseLiteral(b); },
            [](const auto&) { return ParseOptions::finish_single; });
        return result;
    }

    template<class ValueType, class ContextBase, class Token>
    [[nodiscard]] static auto makeTokenValue(
        BlockLineView&, const Token& token, ContextWithTupleLookup<ContextBase>& context) -> Value {
        auto type = context.intrinsicType(meta::Type<ValueType>{});
        if (type == nullptr) {
            assert(type); // this has to resolve a valid API type!
        }
        auto value = Value{type};
        value.set<ValueType>() = ValueType{token};
        return value;
    }

    [[nodiscard]] static auto lookupModule(const strings::View& id, const OptValueExpr& result)
        -> instance::ConstEntryRange {
        return result.map([&](const ValueExpr& n) -> instance::ConstEntryRange {
            return n.visit(
                [&](const ModuleReference& ref) { return ref.module->locals.byName(id); },
                // [&](const VariableReference& ref) {},
                // [&](const ParameterReference& ref) {},
                // [&](const NameTypeValueReference& ref) {},
                [](const auto&) { return instance::ConstEntryRange{}; });
        });
    }

    template<class Context>
    [[nodiscard]] static auto parseInstance( //
        OptValueExpr& result,
        const instance::ConstEntryRange& range,
        BlockLineView& it,
        Context& context) -> ParseOptions {

        return range.frontValue().visit(
            [&](const instance::VariablePtr& var) {
                if (result) return ParseOptions::finish_single;
                result = OptValueExpr{VariableReference{var.get()}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::FunctionPtr& fun) {
                ++it;
                return parseCall(result, fun, it, context);
            },
            [&](const instance::TypePtr& type) {
                if (result) return ParseOptions::finish_single;
                result = OptValueExpr{TypeReference{type.get()}};
                ++it;
                return ParseOptions::continue_single;
            },
            [&](const instance::ModulePtr& mod) {
                if (result) return ParseOptions::finish_single;
                result = OptValueExpr{ModuleReference{mod.get()}};
                ++it;
                return ParseOptions::continue_single;
            });
        // TODO(arBmind): add overloads
    }

    [[nodiscard]] static bool canImplicitConvertToType(ValueExprView node, const parser::TypeView& type) {
        // TODO(arBmind):
        // I guess we need a scope here
        (void)node;
        (void)type;
        return true;
    }

    static void assignLeftArguments(CallOverloads& co, const OptValueExpr& left) {
        auto leftView = left //
            ? left.value().holds<NameTypeValueTuple>() //
                ? ViewNameTypeValueTuple{left.value().get<NameTypeValueTuple>()}
                : ViewNameTypeValueTuple{left.value()}
            : ViewNameTypeValueTuple{};
        for (auto& coi : co.items) {
            auto o = 0U;
            auto t = 0U;
            auto lp = coi.function->leftParameters();
            for (const ViewNameTypeValue& ntv : leftView.tuple) {
                if (!ntv.value) {
                    // pass type?
                }
                else if (ntv.name) {
                    auto optParam = coi.function->lookupParameter(ntv.name.value());
                    if (optParam) {
                        instance::ParameterView param = optParam.value();
                        if (param->side == instance::ParameterSide::left //
                            && canImplicitConvertToType(ntv.value.value(), param->variable->type)) {
                            t++;
                            continue;
                        }
                        // side does not match
                        // type does not match
                    }
                    // name not found
                }
                else if (o < lp.size()) {
                    const auto& param = lp[o];
                    if (param->side == instance::ParameterSide::left //
                        && canImplicitConvertToType(ntv.value.value(), param->variable->type)) {
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
            if (o + t == lp.size()) return;
            // not right count
            coi.active = false;
        }
    }

    template<class Context>
    struct Wrap {
        template<class Type>
        [[nodiscard]] auto intrinsicType(meta::Type<Type>) -> instance::TypeView {
            return context->intrinsicType(meta::Type<Type>{});
        }
        void reportDiagnostic(diagnostic::Diagnostic diagnostic) {
            return context->reportDiagnostic(std::move(diagnostic));
        }
        [[nodiscard]] auto parserForType(const TypeView& type) {
            return [this, parser = Parser::parserForType<Context>(type)](BlockLineView& blv) { //
                return parser(blv, *context);
            };
        }
        template<class Callback>
        [[nodiscard]] auto parseNtvWithCallback(BlockLineView& it, Callback&& cb) -> OptNameTypeValue {
            return Parser::parseNameTypeValueCallback(it, *context, cb);
        }
        Context* context;
    };

    template<class Context>
    [[nodiscard]] static auto parseCall( //
        OptValueExpr& left,
        const instance::FunctionPtr& fun,
        BlockLineView& it,
        Context& context) -> ParseOptions { //

        auto co = CallOverloads{};
        co.items.emplace_back(fun.get());
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

    template<class ContextBase>
    [[nodiscard]] static auto buildCallNode(Call&& call, ContextWithTupleLookup<ContextBase>& context) -> OptValueExpr {
        if (isDirectlyExecutable(call)) {
            return context.runCall(call);
        }
        return ValueExpr{std::move(call)};
    }

    template<class Token, class Context>
    struct TokenVisitor {
        BlockLineView& it;
        Context& context;
        using BlockToken = Token;

        auto operator()(const BlockToken& t) -> OptValueExpr {
            auto result = OptValueExpr{makeTokenValue<Token>(it, t, context)};
            ++it;
            return result;
        }
    };
    template<class Context, class... T>
    constexpr static auto buildTokenVisitors(BlockLineView& it, Context& context, meta::TypeList<T...> = {}) {
        return meta::Overloaded{TokenVisitor<T, Context>{it, context}...};
    }

    template<class Context>
    [[nodiscard]] static auto parseSingleToken(BlockLineView& it, Context& context) -> OptValueExpr {
        return it.current().visit(
            buildTokenVisitors<Context, BlockLiteral, StringLiteral, NumberLiteral, IdentifierLiteral>(it, context),
            [](const auto&) { return OptValueExpr{}; });
    }

    template<class Context>
    using ParseFunc = auto (*)(BlockLineView& it, Context& context) -> OptValueExpr;

    [[nodiscard]] static auto getParserForType(const parser::TypeView& type) -> TypeParser {
        return TypeParser::Expression;
        //        return type.visit(
        //            [&](const Pointer& ptr) {
        //                return ptr.target->visit(
        //                    [&](const TypeInstance& inst) { return inst.concrete->parser; },
        //                    [](const auto&) { return instance::Parser::Expression; });
        //            },
        //            [](const auto&) { return instance::Parser::Expression; });
    }

    template<class Context>
    [[nodiscard]] static auto parserForType(const parser::TypeView& type) -> ParseFunc<Context> {
        using namespace instance;
        switch (getParserForType(type)) {
        case TypeParser::Expression:
            return [](BlockLineView& it, Context& context) {
                return parseValueExpr(it, context); //
            };
        case TypeParser::SingleToken:
            return [](BlockLineView& it, Context& context) { //
                return parseSingleToken(it, context);
            };
        case TypeParser::IdTypeValue:
            return [](BlockLineView& it, Context& context) -> OptValueExpr {
                auto optNtv = parseNameTypeValue(it, context);
                if (optNtv) {
                    auto type = context.intrinsicType(meta::Type<NameTypeValue>{});
                    auto value = NameTypeValue{optNtv.value()};
                    // return {Value{std::move(value), {TypeInstance{type}}}};
                    return {}; // TODO(arBmind): new types
                }
                // return Node{NameTypeValueTuple{{optNtv.value()}}}; // TODO(arBmind): store as value
                return {};
            };

        default: return [](BlockLineView&, Context&) { return OptValueExpr{}; };
        }
    }
};

} // namespace parser
