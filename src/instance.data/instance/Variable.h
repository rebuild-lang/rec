#pragma once
#include "parser/Type.h"

#include "meta/Flags.h"
#include "strings/View.h"

namespace instance {

using Name = strings::String;
using parser::TypeView;

enum class VariableFlag {
    _mutable = 1 << 0, ///< value is allowed to change
    assignable = 1 << 1, ///< variable points outside of the frame
    // based on type and all assignments
    compile_time = 1 << 2, ///< variable is usable at compile time
    run_time = 1 << 3, ///< variable is usable at runtime

    function_parameter = 1 << 4, ///< created by a function parameter
};
using VariableFlags = meta::Flags<VariableFlag>;
META_FLAGS_OP(VariableFlags)

struct Variable {
    Name name;
    TypeView type{}; ///< either concrete type or nullptr
    VariableFlags flags{};
    ParameterView parameter{}; ///< parameter where related to this variable
};
using VariablePtr = std::shared_ptr<Variable>;

} // namespace instance
