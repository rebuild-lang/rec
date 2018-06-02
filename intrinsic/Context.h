#pragma once
#include "instance/Views.h"
#include "parser/expression/Tree.h"

namespace intrinsic {

struct Context {
    instance::Scope* parserScope{};
    const instance::Scope* executionScope{};

    Context(instance::Scope* parserScope, const instance::Scope* executionScope)
        : parserScope(parserScope)
        , executionScope(executionScope) {}

    virtual void parse(const parser::expression::BlockLiteral& block, instance::Scope* scope) const = 0;
};

} // namespace intrinsic
