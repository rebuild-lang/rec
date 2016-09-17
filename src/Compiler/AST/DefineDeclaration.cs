namespace REC.AST
{
    // immutable declaration
    // almost the same as VariableDeclaration but value the final value
    public interface IDefineDeclaration : ITypedDeclaration
    {
        IExpression Value { get; }
    }

    class DefineDeclaration : TypedDeclaration, IDefineDeclaration
    {
        public IExpression Value { get; set; }
    }
}