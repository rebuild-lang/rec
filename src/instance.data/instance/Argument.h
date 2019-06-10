#pragma once
#include "instance/Typed.h"

#include "parser/Tree.h"

#include "meta/Flags.h"
#include "strings/String.h"

namespace instance {

enum class ArgumentSide { left, right, result };
enum class ArgumentFlag {
    optional = 1 << 0,
    splatted = 1 << 1, // array type is gathered during call
    token = 1 << 5, // unparsed token
    expression = 1 << 6, // unevaluated expression
    compile_time = 1 << 7, // compile time result
    assignable = 1 << 8,
    // …
};
using ArgumentFlags = meta::Flags<ArgumentFlag>;
META_FLAGS_OP(ArgumentFlags)

struct Argument {
    Typed typed;
    ArgumentSide side{};
    ArgumentFlags flags{};
    parser::Nodes init{};
};
using Arguments = std::vector<Argument>;

inline auto nameOf(const Argument& arg) -> NameView { return nameOf(arg.typed); }

} // namespace instance
