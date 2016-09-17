namespace REC.AST
{
    // Literals are constants values
    // NumberLiteral, StringLiteral, BlockLiteral
    public interface ILiteral : IExpression
    {
        
    }

    abstract class Literal : Expression, ILiteral
    {
    }

}