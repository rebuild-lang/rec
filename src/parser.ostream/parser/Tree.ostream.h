#pragma once
#include "Type.ostream.h"

#include "parser/Tree.h"

#include "instance/Function.h"
#include "instance/Parameter.h"
#include "instance/Variable.h"

#include "nesting/Token.ostream.h"

#include "strings/join.h"

namespace parser {

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
    return out << (as.parameter ? as.parameter->typed.name : Name("<?>")) << " = " << as.values;
}
inline auto operator<<(std::ostream& out, const Call& inv) -> std::ostream& {
    out << (inv.function ? inv.function->name : Name("<?>")) << "(";
    strings::join(out, inv.arguments, ", ");
    return out << ")";
}
inline auto operator<<(std::ostream& out, const VariableReference& vr) -> std::ostream& {
    return out << (vr.variable ? vr.variable->typed.name : Name("<?>"));
}
inline auto operator<<(std::ostream& out, const ParameterReference& pr) -> std::ostream& {
    return out << (pr.parameter ? pr.parameter->typed.name : Name("<?>"));
}
inline auto operator<<(std::ostream& out, const NameTypeValueReference& ntvr) -> std::ostream& {
    return out << (ntvr.nameTypeValue && ntvr.nameTypeValue->name ? ntvr.nameTypeValue->name.value() : Name("<?>"));
}
inline auto operator<<(std::ostream& out, const TypeReference& tr) -> std::ostream& {
    return out << *tr.type; // << (vr.variable ? vr.variable->typed.name : Name("<?>"));
}
inline auto operator<<(std::ostream& out, const ModuleReference& mr) -> std::ostream& {
    return out << (mr.module ? mr.module->name : Name("<?>"));
}
inline auto operator<<(std::ostream& out, const NameTypeValue& t) -> std::ostream& {
    if (t.name) {
        out << t.name.value();
        if (t.type) out << " :" << t.type.value();
        if (t.value) out << " = " << t.value.value();
    }
    else if (t.type) {
        out << " :" << t.type.value();
        if (t.value) out << " = " << t.value.value();
    }
    else if (t.value) {
        out << t.value.value();
    }
    else
        out << "<invalid>";

    return out;
}
inline auto operator<<(std::ostream& out, const NameTypeValueTuple& nt) -> std::ostream& {
    size_t size = nt.tuple.size();
    if (size > 1) out << "(";
    strings::join(out, nt.tuple, ", ");
    if (size > 1) out << ")";
    return out;
}

inline auto operator<<(std::ostream& out, const Value& val) -> std::ostream& {
    if (val.type()) {
        out << "val: [" << *val.type() << "]";
#ifdef VALUE_DEBUG_DATA
        if (val.data() && val.type()->debugDataFunc) {
            out << " = ";
            val.type()->debugDataFunc(out, val.data());
        }
#endif
        return out;
    }
    else {
        return out << "val: <empty>";
    }
}
inline auto operator<<(std::ostream& out, const IntrinsicCall&) -> std::ostream& {
    return out << "<intrinsic>"; //
}
inline auto operator<<(std::ostream& out, const VariableInit& vi) -> std::ostream& {
    return out << vi.variable << " = " << vi.nodes; //
}

inline auto operator<<(std::ostream& out, const Node& n) -> std::ostream& {
    n.visit([&](const auto& a) {
        using OP = auto(std::ostream&, decltype(a))->std::ostream&;
        static_cast<OP*>(operator<<)(out, a);
    });
    return out;
}

} // namespace parser
