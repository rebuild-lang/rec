using REC.Intrinsic;

namespace REC.AST
{
    public interface IIntrinsicExpression : IExpression
    {
        IIntrinsic Intrinsic { get; }
    }

    class IntrinsicExpression : Expression, IIntrinsicExpression
    {
        public IIntrinsic Intrinsic { get; set; }
    }

}