using System;
using System.Collections.Generic;
using System.IO;
using REC.AST;
using REC.Intrinsic;
using REC.Tools;

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
            return typedReference.Declaration.Name;
        }

        static void Dynamic(IFunctionDeclaration functionDeclaration, ICppScope scope) {
            DeclareFunction(functionDeclaration, scope);
        }

        static void Dynamic(IIntrinsicExpression intrinsicExpression, ICppScope scope) {
            var func = intrinsicExpression.Intrinsic as IFunctionIntrinsic;
            if (func != null) func.GenerateCpp(new CppIntrinsic {Scope = scope});
        }

        static string Dynamic(INamedExpressionTuple expressionTuple, ICppScope scope) {
            string result = null;
            foreach (var sub in expressionTuple.Tuple) result = Dynamic((dynamic) sub.Expression, scope);
            return result;
        }

        static string Dynamic(IFunctionInvocation functionInvocation, ICppScope scope) {
            var function = functionInvocation.Function;
            if (function.Implementation.Expressions.Count == 1 && function.Implementation.Expressions.First() is IIntrinsicExpression)
                scope.EnsureGlobal(function.Name, () => { return CreateGlobalDeclaration(scope, subScope => DeclareFunction(function, subScope)); });

            var leftArgs = BuildArgumentValues(function, function.LeftArguments, functionInvocation.Left, scope, kind: "Left");
            var rightArgs = BuildArgumentValues(function, function.RightArguments, functionInvocation.Right, scope, kind: "Right");
            var resultName = string.Empty;
            var resultArgs = BuildResultValues(function, scope, ref resultName);
            scope.Runtime.AddLine(BuildInvocation(function.Name, leftArgs, rightArgs, resultArgs));
            return resultName;
        }

        static string BuildInvocation(string function, string leftArgs, string rightArgs, string resultArgs) {
            var left = leftArgs.IsEmpty() ? "" : $"std::move({leftArgs})";
            var right = rightArgs.IsEmpty() ? "" : (leftArgs.IsEmpty() ? "" : ", ") + $"std::move({rightArgs})";
            return $"{resultArgs}{function}({left}{right});";
        }

        static string BuildResultValues(IFunctionDeclaration function, ICppScope scope, ref string name) {
            var argumentDeclarations = function.Results;
            if (argumentDeclarations.IsEmpty()) return string.Empty;
            name = scope.MakeLocalName();
            var typeName = GetFunctionTypeName(function.Name, kind: "Result");
            return $"{typeName} {name} = ";
        }

        static string GetFunctionTypeName(string function, string kind) {
            return $"_rebuild_{function}_{kind}";
        }

        static string BuildArgumentValues(
            IFunctionDeclaration function,
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
                scope.Runtime.AddLine($"{name}.{argument.Name} = {value};");
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

            scope.Declaration.AddLine($"inline {resultType} {function.Name}({left}{right}) {{");
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
            foreach (var result in results) scope.Runtime.AddLine($"{resultLocal}.{result.Name} = std::move({result.Name});");
        }

        static void MakeResultLocals(IEnumerable<IArgumentDeclaration> results, ICppScope scope) {
            foreach (var result in results) {
                var resultType = GetArgumentTypeName(result.Type);
                scope.Runtime.AddLine($"{resultType} {result.Name};"); // TODO: initialize to default value
            }
        }

        static void MakeArgumentLocals(IEnumerable<IArgumentDeclaration> arguments, string arg, ICppScope scope) {
            foreach (var argument in arguments) {
                var argumentType = GetArgumentTypeName(argument.Type);
                scope.Runtime.AddLine($"{argumentType} {argument.Name} = std::move({arg}.{argument.Name});");
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
                        innerScope.Declaration.AddLine($"{argTypeName} {argument.Name};");
                        if (arguments.Count == 1) innerScope.Declaration.AddLine($"operator {argTypeName}() const {{ return {argument.Name}; }}");
                    }
                });
            scope.Declaration.AddLine(line: "};");
        }

        static string GetArgumentTypeName(IModuleDeclaration argumentType) {
            return "uint64_t"; // TODO: make this real
        }
    }
}
