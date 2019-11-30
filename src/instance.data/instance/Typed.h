#pragma once
#include "parser/Type.h"

#include "strings/View.h"

namespace instance {

using Name = strings::String;
using NameView = strings::View;
using parser::TypeView;

// common attributes for Parameter & Variable
struct Typed {
    Name name;
    TypeView type{};
};

inline auto nameOf(const Typed& typed) -> NameView { return typed.name; }

} // namespace instance
