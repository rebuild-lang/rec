#pragma once
#include "parser/TypeTree.h"

#include "strings/View.h"

namespace instance {

using Name = strings::CompareView;
using TypeExpression = parser::TypeExpression;

// common attributes for Argument & Variable
struct Typed {
    Name name;
    TypeExpression type{};
};

inline auto nameOf(const Typed& typed) -> const Name& { return typed.name; }

} // namespace instance
