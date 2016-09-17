using System.Collections.Generic;
using REC.Tools;

namespace REC.AST
{
    // backbone of the AST, that gets the work done
    public interface IFunctionInvocation : IExpression
    {
        IFunctionDeclaration Function { get; }
        ICollection<IExpression> Left { get; }
        ICollection<IExpression> Right { get; }
    }

    public interface IArgumentAssignment : IExpression, INamed
    {
        IArgumentDeclaration Variable { get; }
        IExpression Value { get; }
    }

    class FunctionInvocation : Expression, IFunctionInvocation
    {
        public IFunctionDeclaration Function { get; set; }
        public ICollection<IExpression> Left { get; set; }
        public ICollection<IExpression> Right { get; set; }
    }

    class ArgumentAssignment : Expression, IArgumentAssignment
    {
        public string Name => Variable?.Name;
        public IArgumentDeclaration Variable { get; set; }
        public IExpression Value { get; set; }
    }
}