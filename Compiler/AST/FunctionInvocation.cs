//using REC.Tools;

namespace REC.AST
{
    // backbone of the AST, that gets the work done
    public interface IFunctionInvocation : IExpression
    {
        IFunctionDeclaration Function { get; }
        INamedExpressionTuple Left { get; }
        INamedExpressionTuple Right { get; }
    }


    class FunctionInvocation : Expression, IFunctionInvocation
    {
        public IFunctionDeclaration Function { get; set; }
        public INamedExpressionTuple Left { get; set; }
        public INamedExpressionTuple Right { get; set; }
    }


    //public interface IArgumentAssignment : IExpression, INamed
    //{
    //    IArgumentDeclaration Variable { get; }
    //    IExpression Value { get; }
    //}
    //class ArgumentAssignment : Expression, IArgumentAssignment
    //{
    //    public string Name => Variable?.Name;
    //    public IArgumentDeclaration Variable { get; set; }
    //    public IExpression Value { get; set; }
    //}
}
