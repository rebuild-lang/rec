#pragma once
#include "Type.h"

#include "meta/Flags.h"

namespace instance {

enum class VariableFlag {
    _mutable = 1 << 0,
    alias = 1 << 1,
    compile_time = 1 << 2,
    run_time = 1 << 3,
};
using VariableFlags = meta::Flags<VariableFlag>;

struct Variable {
    Name name;
    TypeView type{};
    VariableFlag flags{};
};
using VariableView = const Variable*;

inline auto nameOf(const Variable& var) -> const Name& { return var.name; }

} // namespace instance

META_FLAGS_OP(instance::VariableFlags)
