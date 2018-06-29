#pragma once
#include "StringLiteral.h"

#include "strings/Output.h"

namespace scanner {

auto operator<<(std::ostream& out, const StringLiteralValue& lit) -> std::ostream&;

} // namespace scanner
