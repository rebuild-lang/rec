#pragma once
#include "diagnostic/Diagnostic.h"
#include "instance/Views.h"
#include "parser/Expression.h"

namespace intrinsic {

struct Context {
    instance::Scope* parserScope{};
    const instance::Scope* executionScope{};

    Context(instance::Scope* parserScope, const instance::Scope* executionScope)
        : parserScope(parserScope)
        , executionScope(executionScope) {}

    /// run Parser for a block of code and a custom scope
    virtual auto parse(const parser::BlockLiteral& block, instance::Scope* scope) const -> parser::Block = 0;

    /// report diagnostics from the C++ API
    virtual void report(diagnostic::Diagnostic diagnostic) = 0;
};

} // namespace intrinsic
