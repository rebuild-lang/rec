namespace REC.AST
{
    // Literal are constants values
    public interface ILiteral : IExpression
    {
        
    }

    internal class Literal : Expression, ILiteral
    {
    }

}