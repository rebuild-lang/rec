#pragma once
#include "parser/Expression.h"

#include "instance/Function.h"

namespace parser {

/// Some expressions trigger a side effect at compile time.
///
/// We have to recognize those when we resolve overload sets to prevent multiple different side effects of one
/// expression.

auto hasSideEffects(const ValueExpr&) -> bool;
auto hasSideEffects(const VecOfValueExpr&) -> bool;
inline auto hasSideEffects(const ArgumentAssignment& aa) -> bool { return hasSideEffects(aa.values); }

auto hasSideEffects(const BlockExpr&) -> bool;
auto hasSideEffects(const VecOfBlockExpr&) -> bool;
inline auto hasSideEffects(const Block& block) -> bool { return hasSideEffects(block.expressions); }

auto hasSideEffects(const ArgumentAssignments& aas) -> bool;

auto hasSideEffects(const Call& call) -> bool;

// references themselfs never have side effects
constexpr auto hasSideEffects(const VariableReference&) -> bool { return false; }
constexpr auto hasSideEffects(const TypeReference&) -> bool { return false; }
constexpr auto hasSideEffects(const ModuleReference&) -> bool { return false; }
constexpr auto hasSideEffects(const NameTypeValueReference&) -> bool { return false; }
constexpr auto hasSideEffects(const Value&) -> bool { return false; }

constexpr auto hasSideEffects(const VecOfPartiallyParsed&) -> bool { return false; }

inline auto hasSideEffects(const VariableInit& vi) -> bool { return hasSideEffects(vi.nodes); }
inline auto hasSideEffects(const ModuleInit& mi) -> bool {
    return false;
    // TODO(arBmind): make use of module InitExpr
    // hasSideEffects(mi.nodes);
}

auto hasSideEffects(const NameTypeValueTuple& ntvt) -> bool;
auto hasSideEffects(const NameTypeValue& ntv) -> bool;

// impl
template<class C, class F>
constexpr auto any(const C& c, F&& f) -> bool {
    for (auto& e : c) {
        if (f(e)) return true;
    }
    return false;
}
constexpr auto has_side_effects_call = [](auto& e) -> bool { return hasSideEffects(e); };

inline auto hasSideEffects(const ValueExpr& node) -> bool { return node.visit(has_side_effects_call); }
inline auto hasSideEffects(const VecOfValueExpr& nodes) -> bool { return any(nodes, has_side_effects_call); }

inline auto hasSideEffects(const BlockExpr& node) -> bool { return node.visit(has_side_effects_call); }
inline auto hasSideEffects(const VecOfBlockExpr& nodes) -> bool { return any(nodes, has_side_effects_call); }

inline auto hasSideEffects(const ArgumentAssignments& aas) -> bool { return any(aas, has_side_effects_call); }
inline auto hasSideEffects(const Call& call) -> bool {
    if (call.function && call.function->flags.all(instance::FunctionFlag::compile_time_side_effects)) return true;
    return hasSideEffects(call.arguments);
}

inline auto hasSideEffects(const NameTypeValueTuple& ntvt) -> bool { return any(ntvt.tuple, has_side_effects_call); }

inline auto hasSideEffects(const NameTypeValue& ntv) -> bool { return ntv.value.map(has_side_effects_call); }

} // namespace parser
