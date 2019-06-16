#pragma once
#include "instance/Function.h"

#include "Parameter.ostream.h"

#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream& out, const FunctionFlags& f) -> std::ostream& {
    out << "flags=[";
    auto labels = std::vector<const char*>{};
    if (f.any(FunctionFlag::compiletime)) labels.push_back("compiletime");
    if (f.any(FunctionFlag::runtime)) labels.push_back("runtime");
    if (f.any(FunctionFlag::compiletime_sideeffects)) labels.push_back("ct_sideeffects");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream& out, ParameterView a) -> std::ostream& { return out << *a; }

inline auto operator<<(std::ostream& out, const ParameterViews& av) -> std::ostream& {
    strings::join(out, av, ", ");
    return out;
}

inline auto operator<<(std::ostream& out, const Function& f) -> std::ostream& {
    return out << "fn " << f.name << '(' << f.parameters << ") " << f.flags;
}

} // namespace instance
