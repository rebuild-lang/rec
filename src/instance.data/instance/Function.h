#pragma once
#include "Argument.h"
#include "Body.h"
#include "LocalScope.h"

#include "instance/Views.h"

#include "meta/Flags.h"
#include "meta/Optional.h"
#include "meta/VectorRange.h"
#include "meta/algorithm.h"
#include "strings/View.h"

namespace instance {

using Name = strings::String;
using NameView = strings::View;

enum class FunctionFlag {
    compile_time = 1 << 0,
    run_time = 1 << 1,
};
using FunctionFlags = meta::Flags<FunctionFlag>;

using OptArgumentView = meta::Optional<meta::DefaultPacked<ArgumentView>>;

using ArgumentViews = std::vector<ArgumentView>;
using ArgumentsRange = meta::VectorRange<ArgumentView const>;

struct Function {
    Name name;
    FunctionFlags flags{};
    // PrecedenceLevel level;
    Body body;
    LocalScope argumentScope;
    ArgumentViews arguments;

    auto lookupArgument(NameView name) const -> OptArgumentView;
    auto leftArguments() const -> ArgumentsRange {
        auto b = arguments.begin();
        auto e = meta::findIf(arguments, [](const auto& a) { return a->side != ArgumentSide::left; });
        return {b, e};
    }
    auto rightArguments() const -> ArgumentsRange {
        auto b = meta::findIf(arguments, [](const auto a) { return a->side == ArgumentSide::right; });
        auto e = std::find_if(b, arguments.end(), [](const auto a) { return a->side != ArgumentSide::right; });
        return {b, e};
    }
    void orderArguments() {
        meta::stableSort(arguments, [](const auto a, const auto b) { return a->side < b->side; });
    }
};

inline auto nameOf(const Function& fun) -> NameView { return fun.name; }

} // namespace instance

META_FLAGS_OP(instance::FunctionFlags)
