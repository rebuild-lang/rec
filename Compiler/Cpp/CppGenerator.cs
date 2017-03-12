using REC.AST;
using REC.Instance;
using REC.Intrinsic;
using REC.Tools;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace REC.Cpp
{
    static class CppGenerator
    {
        public static void Generate(TextWriter writer, IExpressionBlock block) {
            writer.WriteLine(value: "#include <stdint.h>");
            writer.WriteLine(value: "#include <tuple>");
            var scope = new CppScope();
            Dynamic(block, scope);
            foreach (var global in scope.Globals) writer.WriteLine(global.Value);
            writer.WriteLine(scope.Declaration.Build());
            writer.WriteLine(value: "int main(int argc, char** argv) {");
            writer.WriteLine(scope.Runtime.Build());
            writer.WriteLine(value: "  return 0;");
            writer.WriteLine(value: "}");
        }

        static void Dynamic(IExpressionBlock block, ICppScope scope) {
            foreach (var blockExpression in block.Expressions) Dynamic((dynamic) blockExpression, scope);
        }

        // ReSharper disable once UnusedParameter.Local
        static string Dynamic(INumberLiteral numberLiteral, ICppScope scope) {
            return numberLiteral.IntegerPart;
        }

        // ReSharper disable once UnusedParameter.Local
        static string Dynamic(ITypedReference typedReference, ICppScope scope) {
            return typedReference.Instance.Name;
        }

        static void Dynamic(IFunctionDeclaration functionDeclaration, ICppScope scope) {
            DeclareFunction(functionDeclaration, scope);
        }

        static string Dynamic(IVariableDeclaration variableDeclaration, ICppScope scope) {
            DeclareVariable(variableDeclaration, scope);
            return string.Empty;
        }

        static void Dynamic(IIntrinsicExpression intrinsicExpression, ICppScope scope) {
            if (intrinsicExpression.Intrinsic is IFunctionIntrinsic func)
                func.GenerateCpp(new CppIntrinsic {Scope = scope});
        }

        static string Dynamic(INamedExpressionTuple expressionTuple, ICppScope scope) {
            string result = null;
            foreach (var sub in expressionTuple.Tuple) result = Dynamic((dynamic) sub.Expression, scope);
            return result;
        }

        static string Dynamic(IFunctionInvocation invocation, ICppScope scope) {
            var function = invocation.Function;
            var declaration = function.Declaration;
            if (declaration.Implementation.Expressions.Count == 1 &&
                declaration.Implementation.Expressions.First() is IIntrinsicExpression)
                scope.EnsureGlobal(
                    function.Name,
                    () => { return CreateGlobalDeclaration(scope, subScope => DeclareFunction(declaration, subScope)); });

            var leftArgs = BuildArgumentValues(function, declaration.LeftArguments, invocation.Left, scope, kind: "Left");
            var rightArgs = BuildArgumentValues(function, declaration.RightArguments, invocation.Right, scope, kind: "Right");
            var resultName = string.Empty;
            var resultArgs = BuildResultValues(function, scope, ref resultName);
            scope.Runtime.AddLine(BuildInvocation(function.Name, leftArgs, rightArgs, resultArgs));
            return resultName;
        }

        static void DeclareVariable(IVariableDeclaration variable, ICppScope scope) {
            var typeName = GetArgumentTypeName(variable.Type);
            var varName = CppEscape(variable.Name);
            scope.Runtime.AddLine($"{typeName} {varName};");
            if (variable.Value != null) {
                scope.Runtime.AddLine($"{varName} = {Dynamic((dynamic) variable.Value, scope)}");
            }
        }

        static string BuildInvocation(string function, string leftArgs, string rightArgs, string resultArgs) {
            var left = leftArgs.IsEmpty() ? "" : $"std::move({leftArgs})";
            var right = rightArgs.IsEmpty() ? "" : (leftArgs.IsEmpty() ? "" : ", ") + $"std::move({rightArgs})";
            return $"{resultArgs}{CppEscape(function)}({left}{right});";
        }

        static string BuildResultValues(IFunctionInstance function, ICppScope scope, ref string name) {
            var argumentDeclarations = function.Declaration.Results;
            if (argumentDeclarations.IsEmpty()) return string.Empty;
            name = scope.MakeLocalName();
            var typeName = GetFunctionTypeName(function.Name, kind: "Result");
            return $"{typeName} {name} = ";
        }

        static readonly Dictionary<char, string> EscapeMap = new Dictionary<char, string> {
            {'+', "_Add"},
            {'-', "_Minus"},
            {'/', "_Div"},
            {'*', "_Mul"},
            {'_', "__"},
            {'=', "_Eq"},
            {'.', "_Dot"}
        };

        static string CppEscape(string name) {
            return string.Join(separator: "", values: name.Select(c => EscapeMap.Fetch(c, c.ToString())));
        }

        static string GetFunctionTypeName(string function, string kind) {
            return $"_rebuild_{CppEscape(function)}_{kind}";
        }

        static string BuildArgumentValues(
            IFunctionInstance function,
            NamedCollection<IArgumentDeclaration> arguments,
            INamedExpressionTuple expressions,
            ICppScope scope,
            string kind) {
            if (arguments.IsEmpty()) return string.Empty;
            var name = scope.MakeLocalName();
            var typeName = GetFunctionTypeName(function.Name, kind);
            scope.Runtime.AddLine($"{typeName} {name};");
            var argN = 0;
            foreach (var expression in expressions.Tuple) {
                var argument = arguments[argN];
                argN++;
                var value = Dynamic((dynamic) expression.Expression, scope);
                if (argument.IsAssignable) {
                    scope.Runtime.AddLine($"{name}.{CppEscape(argument.Name)} = &{value};");
                }
                else {
                    scope.Runtime.AddLine($"{name}.{CppEscape(argument.Name)} = {value};");
                }
            }
            return name;
        }

        static string CreateGlobalDeclaration(ICppScope localScope, Action<ICppScope> action) {
            var scope = new CppScope {
                Globals = localScope.Globals
            };
            action(scope);
            return scope.Declaration.Build();
        }

        static void DeclareFunction(IFunctionDeclaration function, ICppScope scope) {
            DeclareArguments(function, function.LeftArguments, scope, kind: "Left");
            DeclareArguments(function, function.RightArguments, scope, kind: "Right");
            DeclareArguments(function, function.Results, scope, kind: "Result");
            DeclareFunctionBody(function, scope);
        }

        static void DeclareFunctionBody(IFunctionDeclaration function, ICppScope scope) {
            var noResult = function.Results.IsEmpty();
            var noLeft = function.LeftArguments.IsEmpty();
            var noRight = function.RightArguments.IsEmpty();

            var resultType = noResult ? "void" : GetFunctionTypeName(function.Name, kind: "Result");
            var left = string.Empty;
            var leftLocal = string.Empty;
            var right = string.Empty;
            var rightLocal = string.Empty;
            if (!noLeft) {
                leftLocal = scope.MakeLocalName(hint: "left");
                left = GetFunctionTypeName(function.Name, kind: "Left") + " " + leftLocal;
            }
            if (!noRight) {
                rightLocal = scope.MakeLocalName(hint: "right");
                right = (noLeft ? "" : ", ") + GetFunctionTypeName(function.Name, kind: "Right") + " " + rightLocal;
            }

            scope.Declaration.AddLine($"inline {resultType} {CppEscape(function.Name)}({left}{right}) {{");
            scope.WithDeclarationIndented(
                innerScope => {
                    MakeArgumentLocals(function.LeftArguments, leftLocal, innerScope);
                    MakeArgumentLocals(function.RightArguments, rightLocal, innerScope);
                    MakeResultLocals(function.Results, innerScope);

                    Dynamic(function.Implementation, innerScope);

                    if (!noResult) {
                        var resultLocal = scope.MakeLocalName(hint: "result");
                        innerScope.Runtime.AddLine($"{resultType} {resultLocal};");
                        AssignResultsFromLocals(resultLocal, function.Results, innerScope);
                        innerScope.Runtime.AddLine($"return {resultLocal};");
                    }
                });
            scope.Declaration.AddLine(line: "}");
        }

        static void AssignResultsFromLocals(string resultLocal, IEnumerable<IArgumentDeclaration> results, ICppScope scope) {
            foreach (var result in results) {
                var resultName = CppEscape(result.Name);
                scope.Runtime.AddLine($"{resultLocal}.{resultName} = std::move({resultName});");
            }
        }

        static void MakeResultLocals(IEnumerable<IArgumentDeclaration> results, ICppScope scope) {
            foreach (var result in results) {
                var resultType = GetArgumentTypeName(result.Type);
                var resultName = CppEscape(result.Name);
                scope.Runtime.AddLine($"{resultType} {resultName};"); // TODO: initialize to default value
            }
        }

        static void MakeArgumentLocals(IEnumerable<IArgumentDeclaration> arguments, string arg, ICppScope scope) {
            foreach (var argument in arguments) {
                var argumentType = GetArgumentTypeName(argument.Type);
                var argName = CppEscape(argument.Name);
                if (argument.IsAssignable) {
                    scope.Runtime.AddLine($"{argumentType} &{argName} = *std::move({arg}.{argName});");
                }
                else {
                    scope.Runtime.AddLine($"{argumentType} {argName} = std::move({arg}.{argName});");
                }
            }
        }

        static void DeclareArguments(IFunctionDeclaration function, NamedCollection<IArgumentDeclaration> arguments, ICppScope scope, string kind) {
            if (arguments.IsEmpty()) return;
            var typeName = GetFunctionTypeName(function.Name, kind);
            scope.Declaration.AddLine($"struct {typeName} {{");
            scope.WithDeclarationIndented(
                innerScope => {
                    foreach (var argument in arguments) {
                        var argTypeName = GetArgumentTypeName(argument.Type);
                        var argName = CppEscape(argument.Name);
                        if (argument.IsAssignable && kind != "Result") {
                            innerScope.Declaration.AddLine($"{argTypeName}* {argName};");
                            if (arguments.Count == 1)
                                innerScope.Declaration.AddLine($"operator {argTypeName}() const {{ return *{argName}; }}");
                        }
                        else {
                            innerScope.Declaration.AddLine($"{argTypeName} {argName};");
                            if (arguments.Count == 1)
                                innerScope.Declaration.AddLine($"operator {argTypeName}() const {{ return {argName}; }}");
                        }
                    }
                });
            scope.Declaration.AddLine(line: "};");
        }

        static string GetArgumentTypeName(IModuleInstance argumentType) {
            return "uint64_t"; // TODO: make this real
        }
    }
}
