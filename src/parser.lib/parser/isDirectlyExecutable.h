#pragma once
#include "parser/Expression.h"

#include "instance/Function.h"

namespace parser {

bool isDirectlyExecutable(const NameTypeValue&);
bool isDirectlyExecutable(const ValueExpr&);
bool isDirectlyExecutable(const TypeExpr&);
bool isDirectlyExecutable(const Call&);

// impl
inline bool isDirectlyExecutable(const NameTypeValue& indexNtvs) {
    if (indexNtvs.value && !isDirectlyExecutable(indexNtvs.value.value())) return false;
    if (indexNtvs.type && !isDirectlyExecutable(indexNtvs.type.value())) return false;
    return true;
}

inline bool isDirectlyExecutable(const ValueExpr& node) {
    return node.visit(
        [](const Block&) { return false; },
        [](const Call& call) { return isDirectlyExecutable(call); },
        [](const VariableReference&) { return false; },
        [](const NameTypeValueReference&) { return true; },
        [](const VariableInit&) { return false; },
        [](const ModuleReference& mr) { return mr.module != nullptr; },
        [](const NameTypeValueTuple& tuple) {
            for (auto& indexNtvs : tuple.tuple)
                if (!isDirectlyExecutable(indexNtvs)) return false;
            return true;
        },
        [](const TypeReference& tr) { return tr.type != nullptr; },
        [](const Value&) { return true; },
        [](const PartiallyParsed&) { return false; });
}
inline bool isDirectlyExecutable(const TypeExpr& node) {
    return node.visit(
        [](const Call& call) { return isDirectlyExecutable(call); },
        [](const VariableReference&) { return false; },
        [](const NameTypeValueReference&) { return true; },
        [](const TypeReference& tr) { return tr.type != nullptr; },
        [](const Value&) { return true; },
        [](const PartiallyParsed&) { return false; });
}

inline bool isDirectlyExecutable(const Call& call) {
    if (call.function->flags.none(instance::FunctionFlag::compile_time)) return false;
    for (auto& arg : call.arguments) {
        for (auto& node : arg.values) {
            if (!isDirectlyExecutable(node)) return false;
        }
    }
    return true;
}

} // namespace parser
