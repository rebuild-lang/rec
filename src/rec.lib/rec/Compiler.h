#pragma once
#include "diagnostic/Diagnostic.h"
#include "execution/Machine.h"
#include "instance/Scope.h"
#include "text/File.h"
#include "text/decodePosition.h"

#include <ostream>

namespace rec {

using TextFile = text::File;
using TextConfig = text::Config;
using InstanceScope = instance::Scope;
using InstanceScopePtr = instance::ScopePtr;
using CompilerCallback = execution::Compiler;
using diagnostic::Diagnostics;

struct Config : TextConfig {
    std::ostream* tokenOutput{};
    std::ostream* blockOutput{};
    std::ostream* diagnosticsOutput{};
    // std::ostream* rebuildOutput{}; // TODO(arBmind): allow to configure stdout used by builtin stdout
};

struct Compiler final {
private:
    Config config;
    InstanceScopePtr globals;
    InstanceScopePtr globalScope;
    CompilerCallback compilerCallback;
    Diagnostics diagnostics;

    auto executionContext(const InstanceScopePtr& parserScope);
    auto parserContext(const InstanceScopePtr& scope);

public:
    Compiler(Config config, InstanceScopePtr globals = {});
    ~Compiler() = default;

    // the compiler captures this in lambdas, therefore no copy or move allowed
    Compiler(const Compiler&) = delete;
    Compiler(Compiler&&) = delete;
    Compiler& operator=(const Compiler&) = delete;
    Compiler& operator=(Compiler&&) = delete;

    // run the compiler
    void compile(const TextFile& file);
};

} // namespace rec
