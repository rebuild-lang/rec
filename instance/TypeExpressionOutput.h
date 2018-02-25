#pragma once
#include "TypeExpression.h"

#include "instance/TypeOutput.h"

#include "strings/Output.h"

namespace instance {
namespace type {

auto operator<<(std::ostream& out, const Expression& e) -> std::ostream&;

inline auto operator<<(std::ostream& out, const Auto&) -> std::ostream& { return out << "auto"; }

inline auto operator<<(std::ostream& out, const Pointer& p) -> std::ostream& { return out << '*' << *p.target; }

inline auto operator<<(std::ostream& out, const Array& a) -> std::ostream& {
    return out << a.count << " of " << *a.element;
}

inline auto operator<<(std::ostream& out, const Instance& i) -> std::ostream& { return out << *i.concrete; }

inline auto operator<<(std::ostream& out, const Expression& e) -> std::ostream& {
    e.visit([&](const auto& v) { out << v; });
    return out;
}

} // namespace type
} // namespace instance
