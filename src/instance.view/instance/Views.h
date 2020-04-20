#pragma once
#include <memory>

namespace instance {

struct Module;
using ModuleView = const Module*;

struct Parameter;
using ParameterView = const Parameter*;

struct Variable;
using VariableView = const Variable*;

struct Function;
using FunctionView = const Function*;

struct Scope;
using ScopePtr = std::shared_ptr<Scope>;

} // namespace instance
