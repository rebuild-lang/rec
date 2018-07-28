#pragma once
#include "Position.h"

#include <ostream>

namespace text {

inline auto operator<<(std::ostream& out, const Line& l) -> std::ostream& { return out << l.v; }

inline auto operator<<(std::ostream& out, const Column& c) -> std::ostream& { return out << c.v; }

inline auto operator<<(std::ostream& out, const Position& p) -> std::ostream& {
    return out << '[' << p.line << ';' << p.column << ']';
}

} // namespace text
