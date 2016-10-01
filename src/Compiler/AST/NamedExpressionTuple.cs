using REC.Tools;

namespace REC.AST
{
    using NamedExpressionCollection = NamedCollection<INamedExpression>;

    public interface INamedExpression : INamed
    {
        // name might be null if expression was not named

        IExpression Expression { get; }
    }

    public interface INamedExpressionTuple : IExpression
    {
        NamedExpressionCollection Tuple { get; }
    }

    struct NamedExpression : INamedExpression
    {
        public string Name { get; set; }
        public IExpression Expression { get; set; }
    }

    class NamedExpressionTuple : Expression, INamedExpressionTuple
    {
        public NamedExpressionCollection Tuple { get; } = new NamedExpressionCollection();
    }
}
