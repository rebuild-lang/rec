namespace REC.AST
{
    public interface IVariableDeclaration : IDeclaration
    {
        IModule Type { get; }
        IExpression Value { get; }
    }

    internal class VariableDeclaration : Declaration, IVariableDeclaration
    {
        public IModule Type { get; set; }
        public IExpression Value { get; set; }
    }
}