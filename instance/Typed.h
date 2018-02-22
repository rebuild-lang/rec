#pragma once
#include "TypeExpression.h"

namespace instance {

// common attributes for Argument & Variable
struct Typed {
    Name name;
    type::Expression type{};
};

inline auto nameOf(const Typed& typed) -> const Name& { return typed.name; }

} // namespace instance
