using REC.Instance;

namespace REC.AST
{
    public interface ITypedValue : IExpression
    {
        IModuleInstance Type { get; }

        // bytes required for the type
        byte[] Data { get; }
    }

    class TypedValue : Expression, ITypedValue
    {
        public IModuleInstance Type { get; set; }
        public byte[] Data { get; set; }
    }
}
