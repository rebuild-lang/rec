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

    virtual auto parse(const parser::BlockLiteral& block, instance::Scope* scope) const -> parser::Block = 0;
};

} // namespace intrinsic
