using REC.AST;

namespace REC.Instance
{
    interface IVariableInstance : ITypedInstance
    {
        IVariableDeclaration Variable { get; }
    }

    class VariableInstance : AbstractTypedInstance, IVariableInstance
    {
        public IVariableDeclaration Variable { get; set; }
        public IDeclaration Declaration => Variable;
        public override ITypedDeclaration TypedDeclaration => Variable;
    }
}
