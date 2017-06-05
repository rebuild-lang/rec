using System;
using System.Collections.Generic;

namespace REC.AST.Tools
{
    using EnterFunc = Func<IExpression, EnterResponse>;
    using UpdateAction = Action<IExpression, NodeState>;
    using LeaveAction = Action<IExpression>;

    [Flags]
    enum EnterResponse
    {
        None,
        OpenBlock = 1 << 0, // enter the block of FunctionDecl, Scopes, Modules or Phases
        OpenDeclarations = 2, // enter the ArgumentDecl of FunctionDecl
        OpenExpression = 4, // enter expression of ArgumentDecl, VariableDecl or Invocation
        OpenFunction = OpenBlock | OpenDeclarations,
    }

    enum NodeState
    {
        Left,
        Right,
        Result,
        Implementation,
        BlockExpression
    }

    interface IWalker
    {
        EnterFunc EnterNode { get; }
        UpdateAction UpdateNode { get; }
        LeaveAction LeaveNode { get; }

        void Walk(IExpression expression);
    }

    class Walker : IWalker
    {
        public EnterFunc EnterNode { get; set; } = _ => EnterResponse.None;
        public UpdateAction UpdateNode { get; set; } = (_, __) => { };
        public LeaveAction LeaveNode { get; set; } = _ => { };

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
                        UpdateNode(function, NodeState.Left);
                        Walk(function.LeftArguments);
                        UpdateNode(function, NodeState.Right);
                        Walk(function.RightArguments);
                        UpdateNode(function, NodeState.Result);
                        Walk(function.Results);
                    }
                    if (response.HasFlag(EnterResponse.OpenBlock)) {
                        UpdateNode(function, NodeState.Implementation);
                        Walk(function.Implementation.Expressions);
                    }
                    break;
                case IFunctionInvocation invocation:
                    if (response.HasFlag(EnterResponse.OpenExpression)) {
                        UpdateNode(invocation, NodeState.Left);
                        Walk(invocation.Left);
                        UpdateNode(invocation, NodeState.Right);
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
