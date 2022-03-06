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
    compile_time = 1u << 0u, ///< marked as usable at compile time
    run_time = 1u << 1u, ///< marked as usable at run time
    compile_time_side_effects = 1u << 2u, ///< will trigger side effects during compile
                                          /// time execution (declare something etc.)
};
using FunctionFlags = meta::Flags<FunctionFlag>;
META_FLAGS_OP(FunctionFlags)

using OptParameterView = meta::Optional<meta::DefaultPacked<ParameterView>>;

using Parameters = std::vector<ParameterPtr>;
using ParameterRange = meta::VectorRange<ParameterPtr const>;

struct Scope;
using ScopePtr = std::shared_ptr<Scope>;

struct Function : std::enable_shared_from_this<Function> {
    Name name{};
    FunctionFlags flags{};
    // PrecedenceLevel level{};
    Body body{};
    LocalScope parameterScope{};
    Parameters parameters{};
    ScopePtr parserScope{}; // only set if more parsing is required

    auto lookupParameter(NameView) const -> OptParameterView;
    auto leftParameters() const -> ParameterRange {
        auto b = parameters.begin();
        auto e = meta::findIf(parameters, [](const auto& a) { return a->side != ParameterSide::left; });
        return {b, e};
    }
    auto rightParameters() const -> ParameterRange {
        auto b = meta::findIf(parameters, [](const auto& a) { return a->side == ParameterSide::right; });
        auto e = std::find_if(b, parameters.end(), [](const auto& a) { return a->side != ParameterSide::right; });
        return {b, e};
    }
    void orderParameters() {
        meta::stableSort(parameters, [](const auto& a, const auto& b) { return a->side < b->side; });
    }

    auto parameterScopePtr() -> LocalScopePtr { return {shared_from_this(), &parameterScope}; }
};
using FunctionPtr = std::shared_ptr<Function>;

} // namespace instance
