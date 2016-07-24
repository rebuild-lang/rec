using System.Collections.Generic;

namespace REC.AST
{
    public interface IBlock : IExpression
    {
        IEnumerable<IExpression> List { get; }
    }

    internal class Block : Expression, IBlock
    {
        public IEnumerable<IExpression> List { get; set; }
    }
}