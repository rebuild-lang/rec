using System;
using System.IO;
using REC.Instance;
using REC.Tools;

namespace REC.AST.Tools
{
    [Flags]
    enum DebugTypes
    {
        None,
        ArgumentDeclarations = 1 << 0,
        ExpressionBlocks = 1 << 1,
        FunctionDeclarations = 1 << 2,
        FunctionImplementations = 1 << 3,
        FunctionInvocations = 1 << 4,
        ModuleDeclarations = 1 << 5,
        PhaseDeclarations = 1 << 6,

        All = ArgumentDeclarations | ExpressionBlocks | FunctionDeclarations | FunctionImplementations | FunctionInvocations | ModuleDeclarations |
            PhaseDeclarations
    }

    interface IDebugOutput
    {
        DebugTypes Types { get; }
        TextWriter Out { get; }

        void Print(IExpression expression);
    }

    class DebugOutput : IDebugOutput
    {
        public DebugTypes Types { get; set; } = DebugTypes.All;
        public TextWriter Out { get; set; } = Console.Out;

        public void Print(IExpression expression) {
            CreateWalker().Walk(expression);
            Out.Write(value: "\n");
        }

        IWalker CreateWalker() {
            const string level = "  ";
            var indentation = string.Empty;

            // ReSharper disable once ImplicitlyCapturedClosure
            void Unindent(int levels = 1) {
                indentation = indentation.Remove(indentation.Length - level.Length * levels);
            }

            // ReSharper disable once ImplicitlyCapturedClosure
            void Indent() {
                indentation += level;
            }

            return new Walker {
                EnterNode = expr => {
                    switch (expr) {
                    case IArgumentDeclaration argument:
                        if (Types.HasFlag(DebugTypes.ArgumentDeclarations)) {
                            Out.Write($"\n{indentation}argument `{argument.Name}` (type: `{GetTypeName(argument.Type)}`)");
                            if (null != argument.Value) {
                                Indent();
                                Out.Write(value: ":");
                            }
                            return EnterResponse.OpenExpression;
                        }
                        break;
                    case IExpressionBlock _:
                        if (Types.HasFlag(DebugTypes.ExpressionBlocks)) {
                            Out.Write(value: "block:");
                            Indent();
                            return EnterResponse.OpenBlock;
                        }
                        break;
                    case IFunctionDeclaration function:
                        if (Types.HasFlag(DebugTypes.FunctionDeclarations)) {
                            Out.Write($"\n{indentation}function `{function.Name}`:");
                            Indent();
                            return Types.HasFlag(DebugTypes.FunctionImplementations) ? EnterResponse.OpenFunction : EnterResponse.OpenDeclarations;
                        }
                        break;
                    case IFunctionInvocation invocation:
                        if (Types.HasFlag(DebugTypes.FunctionInvocations)) {
                            Out.Write($"\n{indentation}invoke `{invocation.Function.Name}`:");
                            Indent();
                            return EnterResponse.OpenExpression;
                        }
                        break;
                    case IIdentifierLiteral identifier:
                        Out.Write($"\n{indentation}id `{identifier.Content}`");
                        break;
                    case IIntrinsicExpression intrinsic:
                        Out.Write($"\n{indentation}intrinsic `{intrinsic.Intrinsic.Name}`");
                        break;
                    case IModuleDeclaration module:
                        if (Types.HasFlag(DebugTypes.ModuleDeclarations)) {
                            Out.Write($"\n{indentation}module `{module.Name}`:");
                            Indent();
                            return EnterResponse.OpenBlock;
                        }
                        break;
                    case IModuleReference reference:
                        Out.Write($"\n{indentation}reference `{reference.Reference.Name}`");
                        break;
                    case INamedExpression named:
                        if (!string.IsNullOrEmpty(named.Name)) {
                            Out.Write($"\n{indentation}named `{named.Name}`:");
                            Indent();
                        }
                        return EnterResponse.OpenExpression;
                    case INamedExpressionTuple tuple:
                        if (!tuple.Tuple.IsEmpty()) {
                            Out.Write($"\n{indentation}tuple:");
                            Indent();
                        }
                        return EnterResponse.OpenExpression;
                    case INumberLiteral number:
                        Out.Write($"\n{indentation}number `{number.IntegerPart}`");
                        break;
                    case IPhaseDeclaration phase:
                        if (Types.HasFlag(DebugTypes.PhaseDeclarations)) {
                            Out.Write($"\n{indentation}phase `{phase.Name}`:");
                            Indent();
                            return EnterResponse.OpenBlock;
                        }
                        break;
                    case IStringLiteral str:
                        Out.Write($"\n{indentation}string `{str.Content}`");
                        break;
                    case ITypedReference reference:
                        Out.Write($"\n{indentation}reference `{reference.Instance.Name}` (type: `{GetTypeName(reference.Type)}`)");
                        break;
                    case ITypedValue value:
                        Out.Write($"\n{indentation}value (type: `{value.Type.Name}`)");
                        break;
                    case IVariableDeclaration variable:
                        Out.Write($"\n{indentation}varable `{variable.Name}` (type: `{GetTypeName(variable.Type)}`)");
                        if (null != variable.Value) {
                            Indent();
                            Out.Write(value: ":");
                        }
                        return EnterResponse.OpenExpression;
                    default:
                        Out.Write($"Unknown Node Type {expr.GetType().FullName}");
                        break;
                    }
                    return EnterResponse.None;
                },
                UpdateNode = (expr, state) => {
                    switch (expr) {
                    case null:
                        //Out.Write($"\n{indentation}");
                        break;

                    case IFunctionDeclaration _:
                        switch (state) {
                        case NodeState.Left:
                            Out.Write($"\n{indentation}left:");
                            Indent();
                            break;
                        case NodeState.Right:
                            Unindent();
                            Out.Write($"\n{indentation}right:");
                            Indent();
                            break;
                        case NodeState.Result:
                            Unindent();
                            Out.Write($"\n{indentation}results:");
                            Indent();
                            break;
                        case NodeState.Implementation:
                            Unindent();
                            Out.Write($"\n{indentation}implementation:");
                            Indent();
                            break;
                        }
                        break;

                    case IFunctionInvocation _:
                        switch (state) {
                        case NodeState.Left:
                            Out.Write($"\n{indentation}left:");
                            Indent();
                            break;
                        case NodeState.Right:
                            Unindent();
                            Out.Write($"\n{indentation}right:");
                            Indent();
                            break;
                        }
                        break;
                    }
                },
                LeaveNode = expr => {
                    switch (expr) {
                    case IArgumentDeclaration argument:
                        if (Types.HasFlag(DebugTypes.ArgumentDeclarations)) {
                            if (null != argument.Value) Unindent();
                        }
                        break;
                    case IExpressionBlock _:
                        if (Types.HasFlag(DebugTypes.ExpressionBlocks)) Unindent();
                        break;
                    case IFunctionDeclaration _:
                        if (Types.HasFlag(DebugTypes.FunctionDeclarations)) Unindent(levels: 2);
                        Out.Write(value: "\n");
                        break;
                    case IFunctionInvocation _:
                        if (Types.HasFlag(DebugTypes.FunctionInvocations)) Unindent(levels: 2);
                        break;
                    case IModuleDeclaration _:
                        if (Types.HasFlag(DebugTypes.ModuleDeclarations)) Unindent();
                        break;
                    case INamedExpression named:
                        if (!string.IsNullOrEmpty(named.Name)) Unindent();
                        break;
                    case INamedExpressionTuple tuple:
                        if (!tuple.Tuple.IsEmpty()) Unindent();
                        break;
                    case IPhaseDeclaration _:
                        if (Types.HasFlag(DebugTypes.PhaseDeclarations)) Unindent();
                        break;
                    case IVariableDeclaration variable:
                        if (null != variable.Value) Unindent();
                        break;
                    }
                }
            };
        }

        static string GetTypeName(IModuleInstance type) {
            return type?.Name ?? string.Empty;
        }
    }
}
