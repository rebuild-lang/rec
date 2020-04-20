#pragma once
#include "instance/Parameter.h"

#include "parser/Type.ostream.h"

#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream& out, const ParameterFlags& f) -> std::ostream& {
    out << "flags=[";
    auto labels = std::vector<const char*>{};
    if (f.any(ParameterFlag::assignable)) labels.push_back("assignable");
    if (f.any(ParameterFlag::splatted)) labels.push_back("splatted");
    if (f.any(ParameterFlag::run_time)) labels.push_back("run_time");
    if (f.any(ParameterFlag::splatted)) labels.push_back("splatted");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream& out, const Parameter& p) -> std::ostream& {
    return out << p.name /*<< p.type*/ << ' ' << p.flags;
}

inline auto operator<<(std::ostream& out, const ParameterPtr& p) -> std::ostream& { return out << *p; }
inline auto operator<<(std::ostream& out, ParameterView p) -> std::ostream& { return out << *p; }

} // namespace instance
