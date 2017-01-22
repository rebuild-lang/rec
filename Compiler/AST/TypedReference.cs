using REC.Instance;

namespace REC.AST
{
    // reference to any typed declaration
    // usages:
    // * reference a variable, argument or define
    public interface ITypedReference : IExpression
    {
        IModuleInstance Type { get; }

        ITypedInstance Instance { get; }
    }

    class TypedReference : Expression, ITypedReference
    {
        public IModuleInstance Type { get; set; }
        public ITypedInstance Instance { get; set; }
    }
}
