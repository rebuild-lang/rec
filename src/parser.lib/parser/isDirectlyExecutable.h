#pragma once
#include "parser/Tree.h"

#include "instance/Function.h"

namespace parser {

bool isDirectlyExecutable(const NameTypeValue&);
bool isDirectlyExecutable(const Node&);
bool isDirectlyExecutable(const Call&);

// impl
inline bool isDirectlyExecutable(const NameTypeValue& typed) {
    if (typed.value && !isDirectlyExecutable(typed.value.value())) return false;
    if (typed.type && !isDirectlyExecutable(typed.type.value())) return false;
    return true;
}

inline bool isDirectlyExecutable(const Node& node) {
    return node.visit(
        [](const Block&) { return false; },
        [](const Call& call) { return isDirectlyExecutable(call); },
        [](const IntrinsicCall&) { return false; },
        [](const ParameterReference&) { return false; },
        [](const VariableReference&) { return false; },
        [](const NameTypeValueReference&) { return true; },
        [](const VariableInit&) { return false; },
        [](const ModuleReference& mr) { return mr.module != nullptr; },
        [](const NameTypeValueTuple& tuple) {
            for (auto& typed : tuple.tuple)
                if (!isDirectlyExecutable(typed)) return false;
            return true;
        },
        [](const TypeReference& tr) { return tr.type != nullptr; },
        [](const Value&) { return true; });
}

inline bool isDirectlyExecutable(const Call& call) {
    if (call.function->flags.none(instance::FunctionFlag::compiletime)) return false;
    for (auto& arg : call.arguments) {
        for (auto& node : arg.values) {
            if (!isDirectlyExecutable(node)) return false;
        }
    }
    return true;
}

} // namespace parser
