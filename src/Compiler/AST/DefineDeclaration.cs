namespace REC.AST
{
    public interface IDefineDeclaration : IDeclaration
    {
        IModule Type { get; }
        IExpression Value { get; }
    }

    internal class DefineDeclaration : Declaration, IDefineDeclaration
    {
        public IModule Type { get; set; }
        public IExpression Value { get; set; }
    }
}