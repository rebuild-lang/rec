#pragma once
#include "instance/Function.h"

#include "Parameter.ostream.h"

#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream& out, const IntrinsicCall&) -> std::ostream& {
    return out << "<intrinsic>"; //
}

inline auto operator<<(std::ostream& out, const FunctionFlags& f) -> std::ostream& {
    out << "flags=[";
    auto labels = std::vector<const char*>{};
    if (f.any(FunctionFlag::compile_time)) labels.push_back("compiletime");
    if (f.any(FunctionFlag::run_time)) labels.push_back("runtime");
    if (f.any(FunctionFlag::compile_time_side_effects)) labels.push_back("ct_sideeffects");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream& out, const Parameters& av) -> std::ostream& {
    strings::join(out, av, ", ");
    return out;
}

inline auto operator<<(std::ostream& out, const Function& f) -> std::ostream& {
    return out << "fn " << f.name << '(' << f.parameters << ") " << f.flags;
}

} // namespace instance
