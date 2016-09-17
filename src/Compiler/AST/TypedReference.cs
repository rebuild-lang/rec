namespace REC.AST
{
    // reference to any typed declaration
    // usages:
    // * reference a variable, argument or define
    public interface ITypedReference : IExpression
    {
        IModuleDeclaration Type { get; }

        IDeclaration Declaration { get; }
    }

    class TypedReference : Expression, ITypedReference
    {
        public IModuleDeclaration Type { get; set; }
        public IDeclaration Declaration { get; set; }
    }
}
