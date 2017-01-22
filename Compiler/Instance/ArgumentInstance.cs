using REC.AST;

namespace REC.Instance
{
    public enum ArgumentSide
    {
        Left,
        Right,
        Result
    }

    public interface IArgumentInstance : ITypedInstance
    {
        ArgumentSide Side { get; }
        IArgumentDeclaration Argument { get; }
    }

    class ArgumentInstance : AbstractTypedInstance, IArgumentInstance
    {
        public ArgumentSide Side { get; set; }
        public IArgumentDeclaration Argument { get; set; }
        public IDeclaration Declaration => Argument;
        public override ITypedDeclaration TypedDeclaration => Argument;
    }
}
