namespace REC.AST
{
    public interface ITypedValue : IExpression
    {
        IModuleDeclaration Type { get; }

        // bytes required for the type
        byte[] Data { get; }
    }

    class TypedValue : Expression, ITypedValue
    {
        public IModuleDeclaration Type { get; set; }
        public byte[] Data { get; set; }
    }
}