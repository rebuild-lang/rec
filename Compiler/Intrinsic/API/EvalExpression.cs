using System.Diagnostics.CodeAnalysis;
using REC.AST;
using REC.Parser;
using REC.Execution;

#pragma warning disable 649

namespace REC.Intrinsic.API
{
    static class EvalExpression
    {
        public static IFunctionIntrinsic Get() {
            return new FunctionIntrinsic {
                Name = ".Eval",
                RightArgumentsType = typeof(RightArguments),
                ResultType = typeof(ResultArguments),
                CompileTimeApi = (left, right, result, context) => EvalCompileTime((RightArguments) right, (ResultArguments) result, context),
                IsCompileTimeOnly = true
            };
        }

        static void EvalCompileTime(RightArguments right, ResultArguments result, IContext context) {
            result.ResultExpression = CompileTime.Execute(right.EvalExpression, context);
        }

        class RightArguments : IRightArguments
        {
            public IExpression EvalExpression;
        }

        [SuppressMessage(category: "ReSharper", checkId: "NotAccessedField.Local")]
        class ResultArguments : IResultArguments
        {
            [ArgumentAssignable] public IExpression ResultExpression;
        }
    }
}
