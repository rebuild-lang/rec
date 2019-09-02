#pragma once
#include "parser/TypeTree.h"

#include "instance/Type.ostream.h"

namespace parser {

auto operator<<(std::ostream& out, const TypeExpression& e) -> std::ostream&;

inline auto operator<<(std::ostream& out, const Auto&) -> std::ostream& { return out << "auto"; }

inline auto operator<<(std::ostream& out, const Pointer& p) -> std::ostream& { return out << '*' << *p.target; }

inline auto operator<<(std::ostream& out, const Array& a) -> std::ostream& {
    return out << a.count << " of " << *a.element;
}

inline auto operator<<(std::ostream& out, const TypeInstance& i) -> std::ostream& {
    if (i.concrete)
        return out << *i.concrete;
    else
        return out << ":<null>";
}

inline auto operator<<(std::ostream& out, const TypeExpression& e) -> std::ostream& {
    e.visit([&](const auto& v) { out << v; });
    return out;
}

} // namespace parser
