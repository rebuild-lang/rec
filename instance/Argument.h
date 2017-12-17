#pragma once
#include "Type.h"

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
    // …
};
using ArgumentFlags = meta::Flags<ArgumentFlag>;

struct Argument {
    Name name;
    TypeView type{};
    ArgumentSide side{};
    ArgumentFlags flags{};
};
using ArgumentView = const Argument *;
using Arguments = std::vector<Argument>;

inline auto nameOf(const Argument &arg) -> const Name & { return arg.name; }

} // namespace instance

META_FLAGS_OP(instance::ArgumentFlag);
