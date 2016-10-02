using System;
using System.Collections.Generic;
using System.IO;
using REC.AST;
using REC.Intrinsic;
using REC.Tools;

namespace REC.Cpp
{
    using ConceptDict = Dictionary<string, string>;
    using GenerateFunc = Func<string>;

    public interface ICppScope : IDisposable
    {
        ConceptDict Globals { get; }
        string MakeLocalName(string hint = "temp");
        void OutputLine(string line);
        void EnsureGlobal(string concept, GenerateFunc func);
        ICppScope Indent(string text = "  ");
    }

    class CppScope : ICppScope
    {
        public ConceptDict Globals { get; set; }
        public string Indentation { get; set; } = string.Empty;

        public IList<string> Lines { get; set; } = new List<string>();

        public int LocalNameCount { get; set; }

        public string MakeLocalName(string hint = "temp") {
            return $"_rebuild_{hint}{LocalNameCount++}";
        }

        public void OutputLine(string line) {
            Lines.Add(Indentation + line);
        }

        public void EnsureGlobal(string concept, GenerateFunc func) {
            if (Globals.ContainsKey(concept)) return;
            Globals[concept] = func();
        }

        public ICppScope Indent(string text = "  ") {
            return new CppScope {
                Globals = Globals,
                Lines = Lines,
                Indentation = Indentation + text,
                LocalNameCount = LocalNameCount,
            };
        }

        public void Dispose() {
            
        }
    }

    // used for intrinsic function generation
    public interface ICppIntrinsic : ICppScope
    {
        string LeftArgument(string name);
        string RightArgument(string name);
        string ResultArgument(string name);
    }

    class CppIntrinsic : CppScope, ICppIntrinsic
    {
        public static ICppIntrinsic Make(ICppScope iscope) {
            var scope = (CppScope) iscope;
            return new CppIntrinsic {
                Globals = scope.Globals,
                Indentation = scope.Indentation,
                Lines = scope.Lines,
                LocalNameCount = scope.LocalNameCount
            };
        }

        public string LeftArgument(string name) {
            return name;
        }

        public string RightArgument(string name) {
            return name;
        }

        public string ResultArgument(string name) {
            return name;
        }
    }


    public interface ICppGenerator
    {
        void Generate(TextWriter writer, IExpressionBlock block);
    }

    class CppGenerator : ICppGenerator
    {
        ConceptDict _globals = new ConceptDict();

        public void Generate(TextWriter writer, IExpressionBlock block) {
            writer.WriteLine("#include <stdint.h>");
            writer.WriteLine("#include <tuple>");
            string blockString = GenerateBlock(block);
            foreach (var global1 in _globals) {
                writer.WriteLine(global1.Value);
            }
            //writer.WriteLine(blockString);
            writer.WriteLine(value: "int main(int argc, char** argv) {");
            writer.WriteLine(blockString);
            writer.WriteLine(value: "  return 0;");
            writer.WriteLine(value: "}");
        }

        string GenerateBlock(IExpressionBlock block) {
            var lines = new List<string>();
            var scope = new CppScope {Globals = _globals, Indentation = string.Empty, Lines = lines};
            Dynamic(block, scope);
            return  string.Join(separator: "\n", values: lines);
        }

        // ReSharper disable once UnusedParameter.Local
        static string Dynamic(INumberLiteral numberLiteral, ICppScope scope) {
            return numberLiteral.IntegerPart;
        }

        static void Dynamic(IExpressionBlock block, ICppScope scope) {
            foreach (var blockExpression in block.Expressions) {
                Dynamic((dynamic)blockExpression, scope);
            }
        }

        static void Dynamic(IIntrinsicExpression intrinsicExpression, ICppScope scope) {
            var func = intrinsicExpression.Intrinsic as IFunctionIntrinsic;
            if (func != null) {
                func.GenerateCpp(CppIntrinsic.Make(scope));
            }
        }

        static string Dynamic(INamedExpressionTuple expressionTuple, ICppScope scope) {
            string result = null;
            foreach (var sub in expressionTuple.Tuple) {
                result = Dynamic((dynamic)sub.Expression, scope);
            }
            return result;
        }

        static string Dynamic(IFunctionInvocation functionInvocation, ICppScope scope) {
            var function = functionInvocation.Function;
            if (function.Implementation.Expressions.Count == 1 && function.Implementation.Expressions.First() is IIntrinsicExpression) {
                scope.EnsureGlobal(function.Name, () => CreateGlobalDeclaration(scope, subScope => DeclareFunction(function, subScope)));
            }

            var leftArgs = BuildArgumentValues(function, function.LeftArguments, functionInvocation.Left, scope, kind: "Left");
            var rightArgs = BuildArgumentValues(function, function.RightArguments, functionInvocation.Right, scope, kind: "Right");
            var resultArgs = BuildResultValues(function, scope);
            scope.OutputLine(BuildInvocation(function.Name, leftArgs, rightArgs, resultArgs));
            return resultArgs;
        }

        static string BuildInvocation(string function, string leftArgs, string rightArgs, string resultArgs) {
            var left = leftArgs.IsEmpty() ? "" : $"std::move({leftArgs})";
            var right = rightArgs.IsEmpty() ? "" : (leftArgs.IsEmpty() ? "" : ", ") + $"std::move({rightArgs})";
            return $"{resultArgs}{function}({left}{right});";
        }

        static string BuildResultValues(IFunctionDeclaration function, ICppScope scope) {
            var argumentDeclarations = function.Results;
            if (argumentDeclarations.IsEmpty()) return string.Empty;
            var name = scope.MakeLocalName();
            var typeName = GetFunctionTypeName(function.Name, kind: "Result");
            return $"{typeName} {name} = ";
        }

        static string GetFunctionTypeName(string function, string kind) {
            return $"_rebuild_{function}_{kind}";
        }

        static string BuildArgumentValues(IFunctionDeclaration function, NamedCollection<IArgumentDeclaration> arguments, INamedExpressionTuple expressions, ICppScope scope, string kind) {
            if (arguments.IsEmpty()) return string.Empty;
            var name = scope.MakeLocalName();
            var typeName = GetFunctionTypeName(function.Name, kind);
            scope.OutputLine($"{typeName} {name};");
            var argN = 0;
            foreach (var expression in expressions.Tuple) {
                var argument = arguments[argN];
                argN++;
                var value = Dynamic((dynamic) expression.Expression, scope);
                scope.OutputLine($"{name}.{argument.Name} = {value};");
            }
            return name;
        }

        static string CreateGlobalDeclaration(ICppScope localScope, Action<ICppScope> action) {
            var scope = new CppScope {
                Globals = localScope.Globals
            };
            action(scope);
            return string.Join(separator: "\n", values: scope.Lines);
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
                leftLocal = scope.MakeLocalName("left");
                left = GetFunctionTypeName(function.Name, kind: "Left") + " " + leftLocal;
            }
            if (!noRight) {
                rightLocal = scope.MakeLocalName("right");
                right = (noLeft ? "" : ", ") + GetFunctionTypeName(function.Name, kind: "Right") + " " + rightLocal;
            }

            scope.OutputLine($"inline {resultType} {function.Name}({left}{right}) {{");
            using (var innerScope = scope.Indent()) {
                MakeArgumentLocals(function.LeftArguments, leftLocal, innerScope);
                MakeArgumentLocals(function.RightArguments, rightLocal, innerScope);
                //MakeResultLocals(function.Results, innerScope);

                Dynamic(function.Implementation, innerScope);

                if (!noResult) {
                    var resultLocal = scope.MakeLocalName("result");
                    innerScope.OutputLine($"{resultType} {resultLocal};");
                    // TODO: assign values
                    innerScope.OutputLine($"return {resultLocal};");
                }
            }
            scope.OutputLine(line: "}");
        }

        static void MakeArgumentLocals(NamedCollection<IArgumentDeclaration> arguments, string arg, ICppScope scope) {
            foreach (var argument in arguments) {
                var argumentType = GetArgumentTypeName(argument.Type);
                scope.OutputLine($"{argumentType} {argument.Name} = std::move({arg}.{argument.Name});");
            }
        }

        static void DeclareArguments(IFunctionDeclaration function, NamedCollection<IArgumentDeclaration> arguments, ICppScope scope, string kind) {
            if (arguments.IsEmpty()) return;
            var typeName = GetFunctionTypeName(function.Name, kind);
            scope.OutputLine($"struct {typeName} {{");
            using (var innerScope = scope.Indent()) {
                foreach (var argument in arguments) {
                    var argTypeName = GetArgumentTypeName(argument.Type);
                    innerScope.OutputLine($"{argTypeName} {argument.Name};");
                }
            }
            scope.OutputLine(line: "};");
        }

        static string GetArgumentTypeName(IModuleDeclaration argumentType) {
            return "uint64_t"; // TODO: make this real
        }
    }
}