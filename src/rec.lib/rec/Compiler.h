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
using CompilerCallback = execution::Compiler;
using diagnostic::Diagnostics;

struct Config : TextConfig {
    std::ostream* tokenOutput{};
    std::ostream* blockOutput{};
    std::ostream* diagnosticsOutput{};
    // std::ostream* rebuildOutput{}; // TODO(arBmind): allow to configure stdout used by builtin stdout
};

class Compiler final {
    Config config;
    InstanceScope globals;
    InstanceScope globalScope;
    CompilerCallback compilerCallback;
    Diagnostics diagnostics;

    auto executionContext(InstanceScope& parserScope);
    auto parserContext(InstanceScope& scope);

public:
    Compiler(Config config, InstanceScope globals = {});
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
