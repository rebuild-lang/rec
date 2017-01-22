using REC.Instance;

namespace REC.AST
{
    public interface ITypedDeclaration : IDeclaration
    {
        IModuleInstance Type { get; }
    }

    abstract class TypedDeclaration : Declaration, ITypedDeclaration
    {
        public IModuleInstance Type { get; set; }
    }
}