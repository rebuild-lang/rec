namespace REC.AST
{
    // mutable variable declaration
    // almost the same as DefineDeclaration but Value is only initial value
    public interface IVariableDeclaration : ITypedDeclaration
    {
        // might be null if no initial value was given
        IExpression Value { get; }
    }

    internal class VariableDeclaration : TypedDeclaration, IVariableDeclaration
    {
        public IExpression Value { get; set; }
    }
}