#pragma once
#include "Body.h"
#include "LocalScope.h"
#include "Parameter.h"

#include "instance/Views.h"

#include "meta/Flags.h"
#include "meta/Optional.h"
#include "meta/VectorRange.h"
#include "meta/algorithm.h"
#include "strings/View.h"

#include <set>

namespace instance {

using Name = strings::String;
using NameView = strings::View;

enum class FunctionFlag {
    compiletime = 1u << 0u,
    runtime = 1u << 1u,
    compiletime_sideeffects = 1u << 2u,
};
using FunctionFlags = meta::Flags<FunctionFlag>;
META_FLAGS_OP(FunctionFlags)

using OptParameterView = meta::Optional<meta::DefaultPacked<ParameterView>>;

using ParameterViews = std::vector<ParameterView>;
using ParameterRange = meta::VectorRange<ParameterView const>;

struct Function {
    Name name{};
    FunctionFlags flags{};
    // PrecedenceLevel level{};
    Body body{};
    LocalScope parameterScope{};
    ParameterViews parameters{};

    auto lookupParameter(NameView name) const -> OptParameterView;
    auto leftParameters() const -> ParameterRange {
        auto b = parameters.begin();
        auto e = meta::findIf(parameters, [](const auto& a) { return a->side != ParameterSide::left; });
        return {b, e};
    }
    auto rightParameters() const -> ParameterRange {
        auto b = meta::findIf(parameters, [](const auto a) { return a->side == ParameterSide::right; });
        auto e = std::find_if(b, parameters.end(), [](const auto a) { return a->side != ParameterSide::right; });
        return {b, e};
    }
    void orderParameters() {
        meta::stableSort(parameters, [](const auto a, const auto b) { return a->side < b->side; });
    }
};

inline auto nameOf(const Function& fun) -> NameView { return fun.name; }

} // namespace instance
