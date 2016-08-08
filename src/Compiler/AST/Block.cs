using System.Collections.Generic;

namespace REC.AST
{
    public interface IBlock : IExpression
    {
        ICollection<IExpression> Expressions { get; }
    }

    internal class Block : Expression, IBlock
    {
        public ICollection<IExpression> Expressions { get; } = new List<IExpression>();
    }
}