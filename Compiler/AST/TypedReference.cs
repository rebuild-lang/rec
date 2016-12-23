namespace REC.AST
{
    // reference to any typed declaration
    // usages:
    // * reference a variable, argument or define
    public interface ITypedReference : IExpression
    {
        IModuleDeclaration Type { get; }

        ITypedDeclaration Declaration { get; }
    }

    class TypedReference : Expression, ITypedReference
    {
        public IModuleDeclaration Type { get; set; }
        public ITypedDeclaration Declaration { get; set; }
    }
}
