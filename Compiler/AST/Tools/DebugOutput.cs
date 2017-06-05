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
                Visitor = (expr, @event) => {
                    if (expr == null) return EnterResponse.None;
                    switch (expr) {
                    case IArgumentDeclaration argument:
                        if (Types.HasFlag(DebugTypes.ArgumentDeclarations)) {
                            switch (@event) {
                            case NodeEvent.Enter:
                                Out.Write($"\n{indentation}argument `{argument.Name}` (type: `{GetTypeName(argument.Type)}`)");
                                if (null != argument.Value) {
                                    Indent();
                                    Out.Write(value: ":");
                                }
                                return EnterResponse.OpenExpression;
                            case NodeEvent.Leave:
                                if (null != argument.Value) Unindent();
                                break;
                            }
                        }
                        break;
                    case IExpressionBlock _:
                        if (Types.HasFlag(DebugTypes.ExpressionBlocks)) {
                            switch (@event) {
                            case NodeEvent.Enter:
                                Out.Write(value: "block:");
                                Indent();
                                return EnterResponse.OpenBlock;
                            case NodeEvent.Leave:
                                Unindent();
                                break;
                            }
                        }
                        break;
                    case IFunctionDeclaration function:
                        if (Types.HasFlag(DebugTypes.FunctionDeclarations)) {
                            switch (@event) {
                            case NodeEvent.Enter:
                                Out.Write($"\n{indentation}function `{function.Name}`:");
                                Indent();
                                return Types.HasFlag(DebugTypes.FunctionImplementations)
                                    ? EnterResponse.OpenFunction
                                    : EnterResponse.OpenDeclarations;
                            case NodeEvent.Leave:
                                Unindent(levels: 2);
                                Out.Write(value: "\n");
                                break;
                            case NodeEvent.Left:
                                Out.Write($"\n{indentation}left:");
                                Indent();
                                break;
                            case NodeEvent.Right:
                                Unindent();
                                Out.Write($"\n{indentation}right:");
                                Indent();
                                break;
                            case NodeEvent.Result:
                                Unindent();
                                Out.Write($"\n{indentation}results:");
                                Indent();
                                break;
                            case NodeEvent.Implementation:
                                Unindent();
                                Out.Write($"\n{indentation}implementation:");
                                Indent();
                                break;
                            }
                        }
                        break;
                    case IFunctionInvocation invocation:
                        if (Types.HasFlag(DebugTypes.FunctionInvocations)) {
                            switch (@event) {
                            case NodeEvent.Enter:
                                Out.Write($"\n{indentation}invoke `{invocation.Function.Name}`:");
                                Indent();
                                return EnterResponse.OpenExpression;
                            case NodeEvent.Leave:
                                Unindent();
                                break;
                            case NodeEvent.Left:
                                Out.Write($"\n{indentation}left:");
                                Indent();
                                break;
                            case NodeEvent.Right:
                                Unindent();
                                Out.Write($"\n{indentation}right:");
                                Indent();
                                break;
                            }
                        }
                        break;
                    case IIdentifierLiteral identifier:
                        switch (@event) {
                        case NodeEvent.Enter:
                            Out.Write($"\n{indentation}id `{identifier.Content}`");
                            break;
                        }
                        break;
                    case IIntrinsicExpression intrinsic:
                        switch (@event) {
                        case NodeEvent.Enter:
                            Out.Write($"\n{indentation}intrinsic `{intrinsic.Intrinsic.Name}`");
                            break;
                        }
                        break;
                    case IModuleDeclaration module:
                        if (Types.HasFlag(DebugTypes.ModuleDeclarations)) {
                            switch (@event) {
                            case NodeEvent.Enter:
                                Out.Write($"\n{indentation}module `{module.Name}`:");
                                Indent();
                                return EnterResponse.OpenBlock;
                            case NodeEvent.Leave:
                                Unindent();
                                break;
                            }
                        }
                        break;
                    case IModuleReference reference:
                        switch (@event) {
                        case NodeEvent.Enter:
                            Out.Write($"\n{indentation}reference `{reference.Reference.Name}`");
                            break;
                        }
                        break;
                    case INamedExpression named:
                        if (!string.IsNullOrEmpty(named.Name)) {
                            switch (@event) {
                            case NodeEvent.Enter:
                                Out.Write($"\n{indentation}named `{named.Name}`:");
                                Indent();
                                break;
                            case NodeEvent.Leave:
                                Unindent();
                                break;
                            }
                        }
                        return EnterResponse.OpenExpression;
                    case INamedExpressionTuple tuple:
                        if (tuple.Tuple.IsEmpty()) break;
                        if (tuple.Tuple.Count > 1) {
                            switch (@event) {
                            case NodeEvent.Enter:
                                Out.Write($"\n{indentation}tuple:");
                                Indent();
                                break;
                            case NodeEvent.Leave:
                                Unindent();
                                break;
                            }
                        }
                        return EnterResponse.OpenExpression;
                    case INumberLiteral number:
                        switch (@event) {
                        case NodeEvent.Enter:
                            Out.Write($"\n{indentation}number `{number.IntegerPart}`");
                            break;
                        }
                        break;
                    case IPhaseDeclaration phase:
                        if (Types.HasFlag(DebugTypes.PhaseDeclarations)) {
                            switch (@event) {
                            case NodeEvent.Enter:
                                Out.Write($"\n{indentation}phase `{phase.Name}`:");
                                Indent();
                                return EnterResponse.OpenBlock;
                            case NodeEvent.Leave:
                                Unindent();
                                break;
                            }
                        }
                        break;
                    case IStringLiteral str:
                        switch (@event) {
                        case NodeEvent.Enter:
                            Out.Write($"\n{indentation}string `{str.Content}`");
                            break;
                        }
                        break;
                    case ITypedReference reference:
                        switch (@event) {
                        case NodeEvent.Enter:
                            Out.Write($"\n{indentation}reference `{reference.Instance.Name}` (type: `{GetTypeName(reference.Type)}`)");
                            break;
                        }
                        break;
                    case ITypedValue value:
                        switch (@event) {
                        case NodeEvent.Enter:
                            Out.Write($"\n{indentation}value `{GetValueRepr(value)}` (type: `{value.Type.Name}`)");
                            break;
                        }
                        break;
                    case IVariableDeclaration variable:
                        switch (@event) {
                        case NodeEvent.Enter:
                            Out.Write($"\n{indentation}varable `{variable.Name}` (type: `{GetTypeName(variable.Type)}`)");
                            if (null != variable.Value) {
                                Indent();
                                Out.Write(value: ":");
                            }
                            return EnterResponse.OpenExpression;
                        case NodeEvent.Leave:
                            if (null != variable.Value) Unindent();
                            break;
                        }
                        break;
                    default:
                        Out.Write($"Unknown Node Type {expr.GetType().FullName}");
                        break;
                    }
                    return EnterResponse.None;
                }
            };
        }

        static string GetValueRepr(ITypedValue value) {
            if (null == value.Type) return "<untyped>";
            var toNet = value.Type.GetToNetType();
            if (null == toNet) return "<unknown>";
            var net = toNet(value.Data);
            return net.ToString();
        }

        static string GetTypeName(IModuleInstance type) {
            return type?.Name ?? string.Empty;
        }
    }
}
