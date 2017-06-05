using REC.Tools;

namespace REC.AST
{
    using NamedExpressionCollection = NamedCollection<INamedExpression>;

    public interface INamedExpression : IExpression, INamed
    {
        // name might be null if expression was not named

        IExpression Expression { get; }
    }

    public interface INamedExpressionTuple : IExpression
    {
        NamedExpressionCollection Tuple { get; }
    }

    class NamedExpression : Expression, INamedExpression
    {
        public string Name { get; set; }
        public IExpression Expression { get; set; }
    }

    class NamedExpressionTuple : Expression, INamedExpressionTuple
    {
        public NamedExpressionCollection Tuple { get; } = new NamedExpressionCollection();

        public NamedExpressionTuple() { }

        public NamedExpressionTuple(params IExpression[] expressions) {
            foreach (var e in expressions) {
                Tuple.Add(new NamedExpression {Expression = e});
            }
        }

        public NamedExpressionTuple(string name, IExpression expression) {
            Tuple.Add(new NamedExpression {Name = name, Expression = expression});
        }
    }
}
