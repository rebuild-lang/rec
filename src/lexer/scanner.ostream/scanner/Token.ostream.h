#pragma once
#include "NumberLiteral.ostream.h"
#include "StringLiteral.ostream.h"

#include "scanner/Token.h"

#include "text/TextRange.ostream.h"

namespace scanner {

auto operator<<(std::ostream& out, const Token& v) -> std::ostream&;

inline auto operator<<(std::ostream& out, const IdentifierLiteral&) -> std::ostream& { return out; }
inline auto operator<<(std::ostream& out, const OperatorLiteral&) -> std::ostream& { return out; }

} // namespace scanner
