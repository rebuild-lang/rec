using REC.AST;

namespace REC.Instance
{
    public interface ITypedInstance : IInstance
    {
        ITypedDeclaration TypedDeclaration { get; }
        IModuleInstance Type { get; } // TODO: Type is not always statically evaluated
    }

    abstract class AbstractTypedInstance : AbstractInstance, ITypedInstance
    {
        public override string Name => TypedDeclaration?.Name ?? "";
        public abstract ITypedDeclaration TypedDeclaration { get; }
        public IModuleInstance Type => TypedDeclaration?.Type;
    }
}
