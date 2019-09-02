#pragma once

namespace instance {

struct Module;
using ModuleView = const Module*;

struct Parameter;
using ParameterView = const Parameter*;

struct Variable;
using VariableView = const Variable*;

struct Typed;
using TypedView = const Typed*;

struct Function;
using FunctionView = const Function*;

struct Scope;

struct Type;
using TypeView = const Type*;

} // namespace instance
