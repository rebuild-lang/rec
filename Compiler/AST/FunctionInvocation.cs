//using REC.Tools;

using REC.Instance;

namespace REC.AST
{
    /* backbone of the AST, that gets the work done
     * 
     * This is a fully parsed invocation (if argument types are not fixed it is not parsed fully)
     */
    public interface IFunctionInvocation : IExpression
    {
        IFunctionInstance Function { get; }
        INamedExpressionTuple Left { get; }
        INamedExpressionTuple Right { get; }
    }


    class FunctionInvocation : Expression, IFunctionInvocation
    {
        public IFunctionInstance Function { get; set; }
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
