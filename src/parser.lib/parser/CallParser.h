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

        bool active{true}; // more parsing required
        bool complete{}; // all required arguments (possibly more optional)
        bool hasBlocks{}; // true if one argument was a block
        int sideEffects{}; // count the side effects involved for the overload

        Item(const FunctionView& function)
            : function(function)
            , active(!function->parameters.empty())
            , complete(function->parameters.empty()) {}
    };
    using Items = std::vector<Item>;
    Items items{};
    int sideEffects{}; // the total side effects that were created during parsing
    bool tainted{}; // true if error was already reported

    auto countComplete() const -> int {
        auto c = int{};
        for (auto& i : items) {
            if (i.complete)
                c++;
            else
                break;
        }
        return c;
    }
};

struct CallParser {
    template<class T>
    using External = CallErrorReporter::External<T>;

    template<class T>
    static void parse(CallOverloads& os, BlockLineView& it, T t) {
        auto external = External<T>{t};
        auto withBrackets = it.current().holds<nesting::BracketOpen>();
        if (withBrackets) {
            auto& open = it.current().get<nesting::BracketOpen>();
            ++it; // skip BracketOpen
            parseImpl(os, it, external);
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
        parseImpl(os, it, external);
    }

private:
    using Item = CallOverloads::Item;
    using Items = CallOverloads::Items;
    using ItemIt = Items::iterator;

    template<class T>
    static bool isNameTypeValue(const TypeView& t, External<T>& external) {
        return t == external.intrinsicType(meta::Type<parser::NameTypeValue>{});
    }
    template<class T>
    static bool isBlockLiteral(const TypeView& t, External<T>& external) {
        return t == external.intrinsicType(meta::Type<parser::BlockLiteral>{});
    }
    template<class T>
    static bool isNodeBlockLiteral(const OptValueExpr& node, External<T>& external) {
        if (!node) return false;
        if (!node.value().holds<Value>()) return false;
        auto& value = node.value().get<Value>();
        return isBlockLiteral(value.type(), external);
    }

    template<class T>
    static bool canImplicitConvert(const NameTypeValue& ntv, ParameterView parameter, External<T>& external) {
        if (ntv.type || !ntv.value) {
            return isNameTypeValue(parameter->variable->type, external);
        }
        // TODO(arBmind): allow real conversions
        return ntv.value ? true : false;
    }
    template<class T>
    static auto implicitConvert(const NameTypeValue& ntv, ParameterView parameter, External<T>& external)
        -> VecOfValueExpr {
        if (ntv.type || !ntv.value) {
            auto type = external.intrinsicType(meta::Type<NameTypeValue>{});
            auto value = Value(type);
            value.set<NameTypeValue>() = ntv;
            return {std::move(value)};
        }
        return {ntv.value.value()};
    }

    static void parseOptionalComma(BlockLineView& it) {
        if (it && it.current().holds<nesting::CommaSeparator>()) ++it; // skip CommaSeparator
    }

    template<class T>
    static void parseImpl(CallOverloads& os, BlockLineView& startIt, External<T>& external) {
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
            return params[itemIt->argIndex].get();
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
        auto assignParam = [&](ItemIt& itemIt, BlockLineView& it, const NameTypeValue& nameTypeValue) {
            bool sideEffect = hasSideEffects(nameTypeValue);
            if (sideEffect) os.sideEffects++;
            auto allowMismatch = bool{};
            auto baseIt = itemIt->it;
            auto isNamed = nameTypeValue.name && !nameTypeValue.type;
            while (true) {
                auto optParam = isNamed ? paramByName(itemIt, nameTypeValue.name.value()) : paramByPos(itemIt);
                if (optParam && canImplicitConvert(nameTypeValue, optParam.value(), external)) {
                    auto param = optParam.value();
                    auto as = ArgumentAssignment{};
                    as.parameter = param;
                    as.values = implicitConvert(nameTypeValue, param, external);
                    itemIt->args.push_back(std::move(as));
                    itemIt->it = it;
                    itemIt->argIndex = isNamed && paramByPos(itemIt) != optParam ? -1 : (itemIt->argIndex + 1);
                    if (sideEffect) itemIt->sideEffects++;
                    if (isNodeBlockLiteral(nameTypeValue.value, external)) itemIt->hasBlocks = true;
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

            auto parseValue = [&](NameTypeValue& ntv) {
                auto optParam =
                    (ntv.name && !ntv.type) ? scanParamByName(next, ntv.name.value()) : scanParamByPos(next);
                if (optParam) {
                    auto parseArg = external.parserForType(optParam.value()->variable->type);
                    ntv.value = parseArg(nextIt);
                }
                // invalid Param cannot be parsed
            };
            auto optNtv = external.parseNtvWithCallback(nextIt, parseValue);
            if (next == active) continue; // no params matched
            if (optNtv) {
                assignParam(next, nextIt, optNtv.value());
            }
            else {
                next->active = false;
            }
        }
        startIt =
            std::max_element(itemBegin, itemEnd, [](auto& l, auto& r) { return l.it.index() < r.it.index(); })->it;
        std::stable_partition(itemBegin, itemEnd, [](auto& o) { return o.complete; });
    }
};

} // namespace parser
