namespace REC.AST
{
    // Literals are constants values
    // NumberLiteralType, StringLiteral, BlockLiteral
    public interface ILiteral : IExpression
    {
        
    }

    abstract class Literal : Expression, ILiteral
    {
    }

}