#pragma once
#include "Typed.h"

#include "meta/Flags.h"

namespace instance {

enum class VariableFlag {
    _mutable = 1 << 0,
    alias = 1 << 1,
    compile_time = 1 << 2,
    run_time = 1 << 3,
};
using VariableFlags = meta::Flags<VariableFlag>;
META_FLAGS_OP(VariableFlags)

struct Variable {
    Typed typed;
    VariableFlag flags{};
};
using Variables = std::vector<Variable>;

inline auto nameOf(const Variable& var) -> NameView { return nameOf(var.typed); }

} // namespace instance
