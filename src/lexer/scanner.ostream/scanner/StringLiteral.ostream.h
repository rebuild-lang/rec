#pragma once
#include "scanner/StringLiteral.h"

#include <ostream>

namespace scanner {

auto operator<<(std::ostream& out, const StringLiteralValue& lit) -> std::ostream&;

} // namespace scanner