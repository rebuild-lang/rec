#pragma once
#include "parser/TypeTree.h"

#include "strings/View.h"

namespace instance {

using Name = strings::String;
using NameView = strings::View;
using TypeExpression = parser::TypeExpression;

// common attributes for Argument & Variable
struct Typed {
    Name name;
    TypeExpression type{};
};

inline auto nameOf(const Typed& typed) -> NameView { return typed.name; }

} // namespace instance
