#pragma once
#include "Argument.h"

#include "instance/TypeExpressionOutput.h"

#include "strings/Output.h"
#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream& out, const ArgumentFlags& f) -> std::ostream& {
    out << "flags=[";
    auto labels = std::vector<const char*>{};
    if (f.any(ArgumentFlag::optional)) labels.push_back("optional");
    if (f.any(ArgumentFlag::splatted)) labels.push_back("splatted");
    if (f.any(ArgumentFlag::token)) labels.push_back("token");
    if (f.any(ArgumentFlag::expression)) labels.push_back("expression");
    if (f.any(ArgumentFlag::compile_time)) labels.push_back("compilte_time");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream& out, const Argument& a) -> std::ostream& {
    return out << a.typed.name << a.typed.type << ' ' << a.flags;
}

inline auto operator<<(std::ostream& out, const Arguments& av) -> std::ostream& {
    strings::join(out, av, ", ");
    return out;
}

} // namespace instance
