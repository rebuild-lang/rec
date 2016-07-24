namespace REC.AST
{
    public interface IArgumentDeclaration : IVariableDeclaration
    {
    }

    internal class ArgumentDeclaration : VariableDeclaration, IArgumentDeclaration
    {
    }
}