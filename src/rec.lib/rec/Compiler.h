#pragma once
#include "scanner/tokenize.h"
#include "strings/utf8Decode.h"
#include "text/decodePosition.h"

#include "filter/filterTokens.h"
#include "nesting/nestTokens.h"
#include "parser/Parser.h"

#include "instance/Scope.h"

#include "execution/Machine.h"

#include "api/Context.h"
#include "intrinsic/Adapter.h"
#include "intrinsic/ResolveType.h"

#include "diagnostic/Diagnostic.ostream.h"
#include "nesting/Token.ostream.h"
#include "scanner/Token.ostream.h"

#include <iostream>

namespace rec {

using ParserConfig = text::Config;
using InstanceScope = instance::Scope;
using InstanceNode = instance::Node;
using ExecutionContext = execution::Context;
using CompilerCallback = execution::Compiler;

using StringView = strings::View;
using diagnostic::Diagnostic;
using diagnostic::Diagnostics;
using nesting::BlockLiteral;
using parser::Block;
using parser::Call;
using parser::OptNode;

class Compiler final {
    ParserConfig config;
    InstanceScope globals;
    InstanceScope globalScope;
    Diagnostics diagnostics;

    CompilerCallback compilerCallback;

    auto executionContext(InstanceScope& parserScope);
    auto parserContext(InstanceScope& scope);
    void printDiagnostics() const;

public:
    Compiler(ParserConfig config, InstanceScope&& globals);
    ~Compiler() = default;
    Compiler(const Compiler&) = delete;
    Compiler(Compiler&&) = delete;
    Compiler& operator=(const Compiler&) = delete;
    Compiler& operator=(Compiler&&) = delete;

    void compile(const text::File& file);
};

} // namespace rec
