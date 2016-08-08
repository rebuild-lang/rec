using System.Collections.Generic;

namespace REC.AST
{
    public interface IInvocation : IExpression
    {
        IFunctionDeclaration Function { get; }
        ICollection<IExpression> Left { get; }
        ICollection<IExpression> Right { get; }
    }

    public interface IArgumentAssignment : IExpression
    {
        IArgumentDeclaration Variable { get; }
        IExpression Value { get; }
    }

    class Invocation : Expression, IInvocation
    {
        public IFunctionDeclaration Function { get; set; }
        public ICollection<IExpression> Left { get; set; }
        public ICollection<IExpression> Right { get; set; }
    }

    class ArgumentAssignment : Expression, IArgumentAssignment
    {
        public IArgumentDeclaration Variable { get; set; }
        public IExpression Value { get; set; }
    }
}