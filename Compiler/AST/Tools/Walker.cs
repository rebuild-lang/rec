using System;
using System.Collections.Generic;

namespace REC.AST.Tools
{
    using VisitorFunc = Func<IExpression, NodeEvent, EnterResponse>;

    [Flags]
    enum EnterResponse
    {
        None,
        OpenBlock = 1 << 0, // enter the block of FunctionDecl, Scopes, Modules or Phases
        OpenDeclarations = 2, // enter the ArgumentDecl of FunctionDecl
        OpenExpression = 4, // enter expression of ArgumentDecl, VariableDecl or Invocation
        OpenFunction = OpenBlock | OpenDeclarations,
    }

    enum NodeEvent
    {
        Enter, // node is visited for first time
        Leave, // node is visited for last time
        Left, // left part of expression / arguments
        Right, // right part of expression / arguments
        Result, // result arguments
        Implementation, // function implementation
        BlockExpression,
    }

    interface IWalker
    {
        VisitorFunc Visitor { get; }

        void Walk(IExpression expression);
    }

    class Walker : IWalker
    {
        public VisitorFunc Visitor { get; set; } = (_, __) => EnterResponse.None;

        EnterResponse EnterNode(IExpression expr) {
            return Visitor(expr, NodeEvent.Enter);
        }
        void UpdateNode(IExpression expr, NodeEvent @event) {
            Visitor(expr, @event);
        }
        void LeaveNode(IExpression expr) {
            Visitor(expr, NodeEvent.Leave);
        }

        public void Walk(IExpression expression) {
            if (null == expression) return;
            var response = EnterNode(expression);
            try {
                switch (expression) {
                case IArgumentDeclaration argument:
                    if (response.HasFlag(EnterResponse.OpenExpression)) Walk(argument.Value);
                    break;
                case IExpressionBlock block:
                    if (response.HasFlag(EnterResponse.OpenBlock)) Walk(block.Expressions);
                    break;
                case IFunctionDeclaration function:
                    if (response.HasFlag(EnterResponse.OpenDeclarations)) {
                        UpdateNode(function, NodeEvent.Left);
                        Walk(function.LeftArguments);
                        UpdateNode(function, NodeEvent.Right);
                        Walk(function.RightArguments);
                        UpdateNode(function, NodeEvent.Result);
                        Walk(function.Results);
                    }
                    if (response.HasFlag(EnterResponse.OpenBlock)) {
                        UpdateNode(function, NodeEvent.Implementation);
                        Walk(function.Implementation.Expressions);
                    }
                    break;
                case IFunctionInvocation invocation:
                    if (response.HasFlag(EnterResponse.OpenExpression)) {
                        UpdateNode(invocation, NodeEvent.Left);
                        Walk(invocation.Left);
                        UpdateNode(invocation, NodeEvent.Right);
                        Walk(invocation.Right);
                    }
                    break;
                case IIntrinsicExpression _:
                    break;
                case ILiteral _:
                    break;
                case IModuleDeclaration module:
                    if (response.HasFlag(EnterResponse.OpenBlock)) Walk(module.Block.Expressions);
                    break;
                case IModuleReference _:
                    break;
                case INamedExpression named:
                    if (response.HasFlag(EnterResponse.OpenExpression)) Walk(named.Expression);
                    break;
                case INamedExpressionTuple tuple:
                    if (response.HasFlag(EnterResponse.OpenExpression)) Walk(tuple.Tuple);
                    break;
                case IPhaseDeclaration phase:
                    if (response.HasFlag(EnterResponse.OpenBlock)) Walk(phase.Block.Expressions);
                    break;
                case ITypedReference _:
                    break;
                case ITypedValue _:
                    break;
                case IVariableDeclaration variable:
                    if (response.HasFlag(EnterResponse.OpenExpression)) Walk(variable.Value);
                    break;
                default:
                    throw new InvalidOperationException(message: "unknown AST node type");
                }
            }
            finally {
                LeaveNode(expression);
            }
        }

        void Walk(IEnumerable<IExpression> declarations) {
            foreach (var argument in declarations) Walk(argument);
        }
    }
}
