#pragma once
#include "TypeOutput.h"
#include "Variable.h"

#include "strings/Output.h"
#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream &out, const VariableFlags &f) -> std::ostream & {
    out << "flags=[";
    auto labels = std::vector<const char *>{};
    if (f.hasAny(VariableFlag::_mutable)) labels.push_back("mutable");
    if (f.hasAny(VariableFlag::alias)) labels.push_back("alias");
    if (f.hasAny(VariableFlag::compile_time)) labels.push_back("compile_time");
    if (f.hasAny(VariableFlag::run_time)) labels.push_back("run_time");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream &out, const Variable &v) -> std::ostream & {
    // TODO: print flags
    return out << "var " << v.name << v.type;
}

} // namespace instance
