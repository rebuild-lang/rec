#pragma once
#include "Token.h"

#include "NumberLiteralOutput.h"
#include "StringLiteralOutput.h"
#include "TextRangeOutput.h"

#include "strings/Output.h"

namespace scanner {

auto operator<<(std::ostream& out, const TokenVariant& v) -> std::ostream&;

inline auto operator<<(std::ostream& out, const Token& t) -> std::ostream& { return out << t.range << t.data; }

inline auto operator<<(std::ostream& out, const IdentifierLiteral&) -> std::ostream& { return out; }
inline auto operator<<(std::ostream& out, const OperatorLiteral&) -> std::ostream& { return out; }

} // namespace scanner
