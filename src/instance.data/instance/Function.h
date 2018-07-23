#pragma once
#include "Argument.h"
#include "Body.h"

#include "meta/Flags.h"
#include "meta/VectorRange.h"
#include "meta/algorithm.h"
#include "strings/View.h"

namespace instance {

using Name = strings::CompareView;

enum class FunctionFlag {
    compile_time = 1 << 0,
    run_time = 1 << 1,
};
using FunctionFlags = meta::Flags<FunctionFlag>;
using ArgumentsRange = meta::VectorRange<const Argument>;

struct Function {
    Name name;
    FunctionFlags flags;
    Arguments arguments;
    // PrecedenceLevel level;
    Body body;

    auto lookupArgument(const Name& name) const -> decltype(auto) {
        return meta::findIfOpt(arguments, [&](const auto& a) { return name.isContentEqual(a.typed.name); });
    }
    auto leftArguments() const -> ArgumentsRange {
        auto b = arguments.begin();
        auto e = meta::findIf(arguments, [](const auto& a) { return a.side != ArgumentSide::left; });
        return {b, e};
    }
    auto rightArguments() const -> ArgumentsRange {
        auto b = meta::findIf(arguments, [](const auto& a) { return a.side == ArgumentSide::right; });
        auto e = std::find_if(b, arguments.end(), [](const auto& a) { return a.side != ArgumentSide::right; });
        return {b, e};
    }
    void orderArguments() {
        meta::stableSort(arguments, [](const auto& a, const auto& b) { return a.side < b.side; });
    }
};

inline auto nameOf(const Function& fun) -> const Name& { return fun.name; }

} // namespace instance

META_FLAGS_OP(instance::FunctionFlags)
