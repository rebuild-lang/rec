#pragma once
#include "instance/Parameter.h"

#include "parser/Type.ostream.h"

#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream& out, const ParameterFlags& f) -> std::ostream& {
    out << "flags=[";
    auto labels = std::vector<const char*>{};
    if (f.any(ParameterFlag::optional)) labels.push_back("optional");
    if (f.any(ParameterFlag::splatted)) labels.push_back("splatted");
    if (f.any(ParameterFlag::token)) labels.push_back("token");
    if (f.any(ParameterFlag::expression)) labels.push_back("expression");
    if (f.any(ParameterFlag::compile_time)) labels.push_back("compilte_time");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream& out, const Parameter& a) -> std::ostream& {
    return out << a.typed.name << a.typed.type << ' ' << a.flags;
}

inline auto operator<<(std::ostream& out, const Parameters& av) -> std::ostream& {
    strings::join(out, av, ", ");
    return out;
}

} // namespace instance
