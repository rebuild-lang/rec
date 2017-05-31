using System.Diagnostics.CodeAnalysis;

namespace REC.Intrinsic.API
{
    static class CompilerInstance
    {
        public static IFunctionIntrinsic Get() {
            return new FunctionIntrinsic {
                Name = ".Compiler",
                ResultType = typeof(ResultArguments),
                CompileTime = (left, right, result) => EvalCompileTime((ResultArguments) result),
                IsCompileTimeOnly = true
            };
        }

        static void EvalCompileTime(ResultArguments result) {
            result.Compiler = new Compiler();
        }

        [SuppressMessage(category: "ReSharper", checkId: "NotAccessedField.Local")]
        class ResultArguments : IResultArguments
        {
            [ArgumentAssignable] public Compiler Compiler;
        }
    }
}
