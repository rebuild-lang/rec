#pragma once
#include "Module.h"
#include "parser/Type.h"

namespace instance {

using parser::Type;

constexpr auto nameOfType() -> NameView { return NameView{"type"}; }
inline auto nameOf(const Type&) -> NameView { return nameOfType(); }

} // namespace instance
