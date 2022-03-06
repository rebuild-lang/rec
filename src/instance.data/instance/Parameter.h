#pragma once
#include "parser/Expression.h"

#include "meta/Flags.h"
#include "strings/String.h"
#include "strings/View.h"

namespace instance {

using Name = strings::String;

enum class ParameterSide { left, right, result };
enum class ParameterFlag {
    // _mutable = 1 << 0, // == assignable
    assignable = 1 << 1, // only output parameters are assignable

    // based on type and default assignment
    compile_time = 1 << 2, ///< variable is usable at compile time
    run_time = 1 << 3, ///< variable is usable at runtime

    splatted = 1 << 4, // array values are gathered during call
    // token = 1 << 5, // unparsed token
    // expression = 1 << 6, // unevaluated expression
    // …
};
using ParameterFlags = meta::Flags<ParameterFlag>;
META_FLAGS_OP(ParameterFlags)

struct Parameter {
    Name name;
    parser::TypeExpr type{};
    ParameterSide side{};
    ParameterFlags flags{};
    parser::VecOfValueExpr defaultValue{};
    Variable* variable{}; // mutable pointer to the variable that references this parameter
};
using ParameterPtr = std::shared_ptr<Parameter>;

} // namespace instance
