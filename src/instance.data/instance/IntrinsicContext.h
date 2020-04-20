#pragma once
#include "diagnostic/Diagnostic.h"
#include "instance/Views.h"
#include "parser/Expression.h"

namespace intrinsic {

struct ContextInterface {
    instance::ScopePtr parserScope{};
    const instance::Scope* executionScope{};

    ContextInterface(instance::ScopePtr parserScope, const instance::ScopePtr& executionScope)
        : parserScope(std::move(parserScope))
        , executionScope(executionScope.get()) {}

    /// run Parser for a block of code and a custom scope
    [[nodiscard]] virtual auto parse(const parser::BlockLiteral& block, const instance::ScopePtr& scope) const
        -> parser::Block = 0;

    /// report diagnostics from the C++ API
    virtual void report(diagnostic::Diagnostic diagnostic) = 0;
};

} // namespace intrinsic
