#pragma once
#include "instance/Typed.h"

#include "parser/Tree.h"

#include "meta/Flags.h"
#include "strings/String.h"

namespace instance {

enum class ParameterSide { left, right, result };
enum class ParameterFlag {
    optional = 1 << 0,
    splatted = 1 << 1, // array type is gathered during call
    token = 1 << 5, // unparsed token
    expression = 1 << 6, // unevaluated expression
    compile_time = 1 << 7, // compile time result
    assignable = 1 << 8,
    // …
};
using ParameterFlags = meta::Flags<ParameterFlag>;
META_FLAGS_OP(ParameterFlags)

struct Parameter {
    Typed typed;
    ParameterSide side{};
    ParameterFlags flags{};
    parser::Nodes init{};
};
using Parameters = std::vector<Parameter>;

inline auto nameOf(const Parameter& arg) -> NameView { return nameOf(arg.typed); }

} // namespace instance
