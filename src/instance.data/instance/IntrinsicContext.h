#pragma once
#include "instance/Views.h"
#include "parser/Tree.h"

namespace intrinsic {

struct Context {
    instance::Scope* parserScope{};
    const instance::Scope* executionScope{};

    Context(instance::Scope* parserScope, const instance::Scope* executionScope)
        : parserScope(parserScope)
        , executionScope(executionScope) {}

    virtual void parse(const parser::BlockLiteral& block, instance::Scope* scope) const = 0;
};

} // namespace intrinsic
