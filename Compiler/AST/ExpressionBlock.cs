using System.Collections.Generic;

namespace REC.AST
{
    // Expression Blocks are the result of parsing a TokenBlock
    public interface IExpressionBlock : IExpression
    {
        // Modules, Declarations, Invocations & nested blocks are allowed
        IList<IExpression> Expressions { get; }
    }

    class ExpressionBlock : Expression, IExpressionBlock
    {
        public IList<IExpression> Expressions { get; } = new List<IExpression>();
    }
}
