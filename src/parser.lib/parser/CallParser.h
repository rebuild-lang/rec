#pragma once
#include "CallErrorReporter.h"
#include "LineView.h"
#include "hasSideEffects.h"

#include "instance/Function.h"

namespace parser {

using instance::FunctionView;
using instance::OptParameterView;
using instance::ParameterView;

struct CallOverloads {
    struct Item {
        FunctionView function{};
        BlockLineView it{}; // next argument parsing position
        ArgumentAssignments args{}; // successfully assigned arguments
        int argIndex{}; // next positional argument, -1 for none

        bool active{}; // more parsing required
        bool complete{}; // all required arguments (possibly more optional)
        bool hasBlocks{}; // true if one argument was a block
        int sideEffects{}; // count the side effects involved for the overload
    };
    using Items = std::vector<Item>;
    Items items{};
    int sideEffects{}; // the total side effects that were created during parsing
    bool tainted{}; // true if error was already reported
};

struct CallParser {
    template<class T>
    using External = CallErrorReporter::External<T>;

    template<class T>
    static auto parse(CallOverloads& os, BlockLineView& it, T t) {
        auto external = External<T>{t};
        auto withBrackets = it.current().holds<nesting::BracketOpen>();
        if (withBrackets) {
            auto& open = it.current().get<nesting::BracketOpen>();
            ++it; // skip BracketOpen
            parseArgumentsWithout(os, it, external);
            if (!it || !it.current().holds<nesting::BracketClose>()) {
                if (!os.tainted) {
                    CallErrorReporter::reportMissingClosingBracket(open, it, external);
                    os.tainted = true;
                }
                return;
            }
            ++it; // skip BracketClose
            return;
        }
        return parseArgumentsWithout(os, it, external);
    }

private:
    using Item = CallOverloads::Item;
    using Items = CallOverloads::Items;
    using ItemIt = Items::iterator;

    template<class T>
    static bool isTyped(const TypeExpression& t, External<T>& external) {
        return t.visit(
            [&](const TypeInstance& ti) {
                return ti.concrete == external.intrinsicType(meta::Type<parser::NameTypeValue>{});
            },
            [](const auto&) { return false; });
    }
    template<class T>
    static bool isBlockLiteral(const TypeExpression& t, External<T>& external) {
        return t.visit(
            [&](const TypeInstance& ti) {
                return ti.concrete == external.intrinsicType(meta::Type<parser::BlockLiteral>{});
            },
            [](const auto&) { return false; });
    }
    template<class T>
    static bool isNodeBlockLiteral(const OptNode& node, External<T>& external) {
        if (!node) return false;
        if (!node.value().holds<Value>()) return false;
        auto& value = node.value().get<Value>();
        return isBlockLiteral(value.type(), external);
    }

    template<class T>
    static bool canImplicitConvert(const NameTypeValue& typed, ParameterView parameter, External<T>& external) {
        if (typed.type || !typed.value) {
            return isTyped(parameter->typed.type, external);
        }
        // TODO(arBmind): allow real conversions
        return typed.value ? true : false;
    }
    template<class T>
    static auto implicitConvert(const NameTypeValue& typed, ParameterView parameter, External<T>& external) -> Nodes {
        if (typed.type || !typed.value) {
            auto type = external.intrinsicType(meta::Type<NameTypeValue>{});
            return {Value{NameTypeValue{typed}, {TypeInstance{type}}}};
        }
        return {typed.value.value()};
    }

    static void parseOptionalComma(BlockLineView& it) {
        if (it && it.current().holds<nesting::CommaSeparator>()) ++it; // skip CommaSeparator
    }

    template<class T>
    static void parseArgumentsWithout(CallOverloads& os, BlockLineView& startIt, External<T>& external) {
        for (auto& o : os.items) o.it = startIt;
        auto itemBegin = begin(os.items);
        auto itemEnd = end(os.items);
        auto active = itemEnd;

        auto partitionActive = [&] {
            active = std::stable_partition(itemBegin, active, [](auto& o) { return o.active; });
        };
        auto nextItem = [&] {
            return std::min_element(itemBegin, active, [](auto& l, auto& r) { return l.it < r.it; });
        };
        auto skipItem = [&](ItemIt& itemIt, auto it) {
            for (itemIt++;; itemIt++) {
                if (itemIt == active) return false;
                if (itemIt->it != it) continue;
                return true;
            }
        };

        auto paramByName = [&](const ItemIt& itemIt, strings::View name) -> OptParameterView {
            return itemIt->function->lookupParameter(name);
        };
        auto paramByPos = [&](const ItemIt& itemIt) -> OptParameterView {
            auto params = itemIt->function->rightParameters(); // TODO(arBmind): cache rightParameters!
            if (itemIt->argIndex < 0 || itemIt->argIndex >= params.size()) return {};
            return params[itemIt->argIndex];
        };
        auto scanParamByName = [&](ItemIt& itemIt, strings::View name) -> OptParameterView {
            while (true) {
                auto optParam = paramByName(itemIt, name);
                if (optParam) return optParam;
                itemIt->active = false;
                if (!skipItem(itemIt, itemIt->it)) return {};
            }
        };
        auto scanParamByPos = [&](ItemIt& itemIt) -> OptParameterView {
            while (true) {
                auto optParam = paramByPos(itemIt);
                if (optParam) return optParam;
                itemIt->active = false;
                if (!skipItem(itemIt, itemIt->it)) return {};
            }
        };
        auto updateStatus = [](Item& item) {
            // TODO(arBmind): check for optional arguments
            if (item.args.size() == item.function->rightParameters().size()) {
                item.complete = true;
                item.active = false;
            }
        };
        auto assignParam = [&](ItemIt& itemIt, BlockLineView& it, const NameTypeValue& typed) {
            bool sideEffect = hasSideEffects(typed);
            if (sideEffect) os.sideEffects++;
            auto allowMismatch = bool{};
            auto baseIt = itemIt->it;
            auto isNamed = typed.name && !typed.type;
            while (true) {
                auto optParam = isNamed ? paramByName(itemIt, typed.name.value()) : paramByPos(itemIt);
                if (optParam && canImplicitConvert(typed, optParam.value(), external)) {
                    auto param = optParam.value();
                    auto as = ArgumentAssignment{};
                    as.parameter = param;
                    as.values = implicitConvert(typed, param, external);
                    itemIt->args.push_back(std::move(as));
                    itemIt->it = it;
                    itemIt->argIndex = isNamed ? -1 : (itemIt->argIndex + 1);
                    if (sideEffect) itemIt->sideEffects++;
                    if (isNodeBlockLiteral(typed.value, external)) itemIt->hasBlocks = true;
                    updateStatus(*itemIt);
                    if (itemIt->active) parseOptionalComma(itemIt->it);
                }
                else if (!allowMismatch) {
                    itemIt->active = false;
                }

                allowMismatch = true;
                if (!skipItem(itemIt, baseIt)) return;
            }
        };

        while (true) {
            partitionActive();
            if (active == itemBegin) break; // no active items

            auto next = nextItem();
            auto nextIt = next->it;

            auto parseValue = [&](NameTypeValue& typed) {
                auto optParam =
                    (typed.name && !typed.type) ? scanParamByName(next, typed.name.value()) : scanParamByPos(next);
                if (optParam) {
                    auto parseArg = external.parserForType(optParam.value()->typed.type);
                    typed.value = parseArg(nextIt);
                }
                // invalid Param cannot be parsed
            };
            auto optTyped = external.parseTypedWithCallback(nextIt, parseValue);
            if (next == active) continue; // no params matched
            if (optTyped) {
                assignParam(next, nextIt, optTyped.value());
            }
            else {
                next->active = false;
            }
        }
        startIt =
            std::max_element(itemBegin, itemEnd, [](auto& l, auto& r) { return l.it.index() < r.it.index(); })->it;
    }
};

} // namespace parser
