namespace REC.AST
{
    public interface IExpressionLiteral : ILiteral
    {
        IExpression Expression { get; }
    }

    class ExpressionLiteral : Literal, IExpressionLiteral
    {
        public IExpression Expression { get; set; }
    }
}
