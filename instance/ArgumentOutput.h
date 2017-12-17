#pragma once
#include "Argument.h"

#include "instance/TypeOutput.h"

#include "strings/Output.h"
#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream &out, const ArgumentFlags &f) -> std::ostream & {
    out << "flags=[";
    auto labels = std::vector<const char *>{};
    if (f.hasAny(ArgumentFlag::optional)) labels.push_back("optional");
    if (f.hasAny(ArgumentFlag::splatted)) labels.push_back("splatted");
    if (f.hasAny(ArgumentFlag::token)) labels.push_back("token");
    if (f.hasAny(ArgumentFlag::expression)) labels.push_back("expression");
    if (f.hasAny(ArgumentFlag::compile_time)) labels.push_back("compilte_time");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream &out, const Argument &a) -> std::ostream & {
    return out << a.name << a.type << ' ' << a.flags;
}

inline auto operator<<(std::ostream &out, const Arguments &av) -> std::ostream & {
    strings::join(out, av, ", ");
    return out;
}

} // namespace instance
