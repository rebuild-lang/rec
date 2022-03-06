#pragma once
#include "instance/Variable.h"

#include "parser/Type.ostream.h"

#include "strings/String.ostream.h"
#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream& out, const VariableFlags& f) -> std::ostream& {
    out << "flags=[";
    auto labels = std::vector<const char*>{};
    if (f.any(VariableFlag::_mutable)) labels.push_back("mutable");
    if (f.any(VariableFlag::assignable)) labels.push_back("assignable");
    if (f.any(VariableFlag::compile_time)) labels.push_back("compile_time");
    if (f.any(VariableFlag::run_time)) labels.push_back("run_time");
    if (f.any(VariableFlag::function_parameter)) labels.push_back("function_parameter");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream& out, const Variable& v) -> std::ostream& {
    // TODO(arBmind): print flags
    return out << "var " << v.name << v.type;
}

inline auto operator<<(std::ostream& out, const VariablePtr& v) -> std::ostream& { return out << *v; }
inline auto operator<<(std::ostream& out, VariableView v) -> std::ostream& { return out << *v; }

} // namespace instance
