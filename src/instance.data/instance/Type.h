#pragma once
#include "parser/Type.h"

#include "strings/View.h"

namespace parser {

using NameView = strings::View;

constexpr auto nameOfType() -> NameView { return NameView{"type"}; }
inline auto nameOf(const Type&) -> NameView { return nameOfType(); }

} // namespace parser

namespace instance {

using parser::Type;
using TypePtr = std::shared_ptr<Type>;

} // namespace instance
