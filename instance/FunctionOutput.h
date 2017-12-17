#pragma once
#include "Function.h"
#include "instance/ArgumentOutput.h"

#include "strings/Output.h"
#include "strings/join.h"

namespace instance {

inline auto operator<<(std::ostream &out, const FunctionFlags &f) -> std::ostream & {
    out << "flags=[";
    auto labels = std::vector<const char *>{};
    if (f.hasAny(FunctionFlag::compile_time)) labels.push_back("compile_time");
    if (f.hasAny(FunctionFlag::run_time)) labels.push_back("run_time");
    strings::join(out, labels, ", ");
    return out << ']';
}

inline auto operator<<(std::ostream &out, const Function &f) -> std::ostream & {
    return out << "fn " << f.name << '(' << f.arguments << ") " << f.flags;
}

} // namespace instance
