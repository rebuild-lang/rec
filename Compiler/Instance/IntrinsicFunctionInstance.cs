using REC.AST;
using REC.Intrinsic;

namespace REC.Instance
{
    class IntrinsicFunctionInstance : FunctionInstance
    {
        internal IIntrinsicExpression IntrinsicExpression;

        internal IntrinsicFunctionInstance(IFunctionIntrinsic intrinsic) : base(intrinsic.Name) {
            IntrinsicExpression = new IntrinsicExpression {
                Intrinsic = intrinsic
            };
            Declaration = new FunctionDeclaration {
                Name = intrinsic.Name,
                Implementation = new ExpressionBlock {
                    Expressions = { IntrinsicExpression }
                }
            };
        }
    }
}