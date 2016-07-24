using System.Collections.Generic;

namespace REC.AST
{
    public interface IInvocation : IExpression
    {
        IFunctionDeclaration Function { get; }
        IEnumerable<IExpression> Left { get; }
        IEnumerable<IExpression> Right { get; }
    }

    public interface IArgumentAssignment : IExpression
    {
        IArgumentDeclaration Variable { get; }
        IExpression Value { get; }
    }

    class Invocation : Expression, IInvocation
    {
        public IFunctionDeclaration Function { get; set; }
        public IEnumerable<IExpression> Left { get; set; }
        public IEnumerable<IExpression> Right { get; set; }
    }

    class ArgumentAssignment : Expression, IArgumentAssignment
    {
        public IArgumentDeclaration Variable { get; set; }
        public IExpression Value { get; set; }
    }
}