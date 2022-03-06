#pragma once
#include "Type.ostream.h"

#include "parser/Expression.h"

#include "instance/Function.ostream.h"
#include "instance/Parameter.ostream.h"
#include "instance/Variable.ostream.h"

#include "nesting/Token.ostream.h"

#include "strings/join.h"

namespace parser {

auto operator<<(std::ostream& out, const ValueExpr&) -> std::ostream&;
auto operator<<(std::ostream& out, const BlockExpr&) -> std::ostream&;
auto operator<<(std::ostream& out, const TypeExpr& n) -> std::ostream&;

inline auto operator<<(std::ostream& out, const VecOfValueExpr& ns) -> std::ostream& {
    auto size = ns.size();
    if (size > 1) out << "(";
    strings::join(out, ns, ", ");
    if (size > 1) out << ")";
    return out;
}
inline auto operator<<(std::ostream& out, const VecOfBlockExpr& ns) -> std::ostream& {
    auto size = ns.size();
    if (size > 1) out << "(";
    strings::join(out, ns, ", ");
    if (size > 1) out << ")";
    return out;
}
inline auto operator<<(std::ostream& out, const Block& b) -> std::ostream& {
    if (b.expressions.empty()) {
        out << "{}\n";
    }
    else {
        out << "{\n  ";
        strings::join(out, b.expressions, "\n  ");
        out << "\n}\n";
    }
    return out;
}
inline auto operator<<(std::ostream& out, const IdentifierLiteral& il) -> std::ostream& {
    using OP = auto(std::ostream&, const IdentifierLiteral&)->std::ostream&;
    return static_cast<OP*>(scanner::operator<<)(out, il);
}
inline auto operator<<(std::ostream& out, const ScopedBlockLiteral& sbl) -> std::ostream& {
    using OP = auto(std::ostream&, const BlockLiteral&)->std::ostream&;
    return static_cast<OP*>(nesting::operator<<)(out, sbl.block);
}
inline auto operator<<(std::ostream& out, const VecOfPartiallyParsed& pp) -> std::ostream& {
    out << "PartiallyParsed{";
    strings::join(out, pp, ", ");
    return out << "}";
}
inline auto operator<<(std::ostream& out, const ArgumentAssignment& as) -> std::ostream& {
    return out << (as.parameter ? as.parameter->name : Name("<?>")) << " = " << as.values;
}
inline auto operator<<(std::ostream& out, const Call& inv) -> std::ostream& {
    out << (inv.function ? inv.function->name : Name("<?>")) << "(";
    strings::join(out, inv.arguments, ", ");
    return out << ")";
}
inline auto operator<<(std::ostream& out, const VariableReference& vr) -> std::ostream& {
    return out << (vr.variable ? vr.variable->name : Name("<?>"));
}
inline auto operator<<(std::ostream& out, const NameTypeValueReference& ntvr) -> std::ostream& {
    return out << (ntvr.nameTypeValue && ntvr.nameTypeValue->name ? ntvr.nameTypeValue->name.value() : Name("<?>"));
}
inline auto operator<<(std::ostream& out, const TypeReference& tr) -> std::ostream& {
    return out << *tr.type; // << (vr.variable ? vr.variable->indexNtvs.name : Name("<?>"));
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
inline auto operator<<(std::ostream& out, const VariableInit& vi) -> std::ostream& {
    return out << vi.variable << " = " << vi.nodes; //
}
inline auto operator<<(std::ostream& out, const ModuleInit& vi) -> std::ostream& {
    return out << vi.module << ".init" /*<< " = " << vi.nodes*/; //
}

inline auto operator<<(std::ostream& out, const ValueExpr& n) -> std::ostream& {
    n.visit([&]<class A>(const A& a) {
        using OP = auto(std::ostream&, const A&)->std::ostream&;
        static_cast<OP*>(operator<<)(out, a);
    });
    return out;
}
inline auto operator<<(std::ostream& out, const BlockExpr& n) -> std::ostream& {
    n.visit([&]<class A>(const A& a) {
        using OP = auto(std::ostream&, const A&)->std::ostream&;
        static_cast<OP*>(operator<<)(out, a);
    });
    return out;
}
inline auto operator<<(std::ostream& out, const TypeExpr& n) -> std::ostream& {
    n.visit([&]<class A>(const A& a) {
        using OP = auto(std::ostream&, const A&)->std::ostream&;
        static_cast<OP*>(operator<<)(out, a);
    });
    return out;
}

} // namespace parser
