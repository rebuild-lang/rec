#pragma once
#include "parser/Tree.h"

#include "instance/Function.h"

namespace parser {

/// Some expressions trigger a side effect at compile time.
///
/// We have to recognize those when we resolve overload sets to prevent multiple different side effects of one
/// expression.

bool hasSideEffects(const Node& node);
bool hasSideEffects(const Nodes& nodes);

inline bool hasSideEffects(const Block& block) { return hasSideEffects(block.nodes); }
inline bool hasSideEffects(const ArgumentAssignment& aa) { return hasSideEffects(aa.values); }

bool hasSideEffects(const ArgumentAssignments& aas);

bool hasSideEffects(const Call& call);

// correct flags are transported to the FunctionView
constexpr bool hasSideEffects(const IntrinsicCall&) { return false; }

// references themselfs never have side effects
constexpr bool hasSideEffects(const ParameterReference&) { return false; }
constexpr bool hasSideEffects(const VariableReference&) { return false; }
constexpr bool hasSideEffects(const ModuleReference&) { return false; }
constexpr bool hasSideEffects(const NameTypeValueReference&) { return false; }
constexpr bool hasSideEffects(const Value&) { return false; }

inline bool hasSideEffects(const VariableInit& vi) { return hasSideEffects(vi.nodes); }

bool hasSideEffects(const NameTypeValueTuple& ntvt);
bool hasSideEffects(const NameTypeValue& ntv);

// impl
template<class C, class F>
constexpr bool any(const C& c, F&& f) {
    for (auto& e : c) {
        if (f(e)) return true;
    }
    return false;
}
constexpr auto has_side_effects_call = [](auto& e) -> bool { return hasSideEffects(e); };

inline bool hasSideEffects(const Node& node) { return node.visit(has_side_effects_call); }
inline bool hasSideEffects(const Nodes& nodes) { return any(nodes, has_side_effects_call); }

inline bool hasSideEffects(const ArgumentAssignments& aas) { return any(aas, has_side_effects_call); }
inline bool hasSideEffects(const Call& call) {
    if (call.function && call.function->flags.all(instance::FunctionFlag::compiletime_sideeffects)) return true;
    return hasSideEffects(call.arguments);
}

inline bool hasSideEffects(const NameTypeValueTuple& ntvt) { return any(ntvt.tuple, has_side_effects_call); }

inline bool hasSideEffects(const NameTypeValue& ntv) { return ntv.value.map(has_side_effects_call); }

} // namespace parser
