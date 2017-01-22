using System.Collections.Generic;
using System.Reflection;
using REC.Instance;
using REC.Intrinsic;

namespace REC.AST
{
    using FieldToDecl = Dictionary<FieldInfo, ITypedInstance>;

    public interface IIntrinsicExpression : IExpression
    {
        IFunctionIntrinsic Intrinsic { get; }

        FieldToDecl LeftFields { get; }
        FieldToDecl RightFields { get; }
        FieldToDecl ResultFields { get; }
    }

    class IntrinsicExpression : Expression, IIntrinsicExpression
    {
        public IFunctionIntrinsic Intrinsic { get; set; }

        public FieldToDecl LeftFields { get; } = new FieldToDecl();
        public FieldToDecl RightFields { get; } = new FieldToDecl();
        public FieldToDecl ResultFields { get; } = new FieldToDecl();
    }
}
