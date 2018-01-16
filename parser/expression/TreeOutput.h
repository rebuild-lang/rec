#pragma once
#include "Tree.h"

#include "instance/Argument.h"
#include "instance/Function.h"
#include "instance/Variable.h"

#include "parser/block/TokenOutput.h"
#include "strings/Output.h"
#include "strings/join.h"

namespace parser::expression {

auto operator<<(std::ostream& out, const Node&) -> std::ostream&;
inline auto operator<<(std::ostream& out, const Nodes& ns) -> std::ostream& {
    auto size = ns.size();
    if (size > 1) out << "(";
    strings::join(out, ns, ", ");
    if (size > 1) out << ")";
    return out;
}
inline auto operator<<(std::ostream& out, const Block& b) -> std::ostream& {
    if (b.nodes.empty()) {
        out << "{}\n";
    }
    else {
        out << "{\n  ";
        strings::join(out, b.nodes, "\n  ");
        out << "\n}\n";
    }
    return out;
}
inline auto operator<<(std::ostream& out, const ArgumentAssignment& as) -> std::ostream& {
    return out << (as.argument ? as.argument->name : Name("<?>")) << " = " << as.values;
}
inline auto operator<<(std::ostream& out, const Invocation& inv) -> std::ostream& {
    out << (inv.function ? inv.function->name : Name("<?>")) << "(";
    strings::join(out, inv.arguments, ", ");
    return out << ")";
}
inline auto operator<<(std::ostream& out, const VariableReference& vr) -> std::ostream& {
    return out << (vr.variable ? vr.variable->name : Name("<?>"));
}
inline auto operator<<(std::ostream& out, const Named& n) -> std::ostream& {
    if (n.name.isEmpty()) {
        out << n.node;
    }
    else {
        out << n.name << " = " << n.node;
    }
    return out;
}
inline auto operator<<(std::ostream& out, const NamedTuple& nt) -> std::ostream& {
    size_t size = nt.tuple.size();
    if (size > 1) out << "(";
    strings::join(out, nt.tuple, ", ");
    if (size > 1) out << ")";
    return out;
}
inline auto operator<<(std::ostream& out, const Literal& lit) -> std::ostream& {
    out << "lit: ";
    lit.value.visit([&](const auto& a) { out << a; });
    return out;
}
inline auto operator<<(std::ostream& out, const Node& n) -> std::ostream& {
    n.visit([&](const auto& a) { out << a; });
    return out;
}

} // namespace parser::expression
