#pragma once
#include "Token.h"

#include "NumberLiteralOutput.h"
#include "StringLiteralOutput.h"
#include "TextRangeOutput.h"

namespace scanner {

auto operator<<(std::ostream& out, const Token& v) -> std::ostream&;

inline auto operator<<(std::ostream& out, const IdentifierLiteral&) -> std::ostream& { return out; }
inline auto operator<<(std::ostream& out, const OperatorLiteral&) -> std::ostream& { return out; }

} // namespace scanner
