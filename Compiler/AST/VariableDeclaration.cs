namespace REC.AST
{
    // mutable variable declaration
    // almost the same as DefineDeclaration but Value is only initial value
    public interface IVariableDeclaration : ITypedDeclaration
    {
        // if true allows to be passed in assignments
        bool IsAssignable { get; }

        // might be null if no initial value was given
        IExpression Value { get; }
    }

    class VariableDeclaration : TypedDeclaration, IVariableDeclaration
    {
        public bool IsAssignable { get; set; }
        public IExpression Value { get; set; }
    }
}
