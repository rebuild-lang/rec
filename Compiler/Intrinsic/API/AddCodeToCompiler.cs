using REC.AST;

#pragma warning disable 649

namespace REC.Intrinsic.API
{
    static class AddCodeToCompiler
    {
        public static IFunctionIntrinsic Get() {
            return new FunctionIntrinsic {
                Name = ".AddCode",
                RightArgumentsType = typeof(RightArguments),
                CompileTime = (left, right, result) => EvalCompileTime((RightArguments) right),
                IsCompileTimeOnly = true
            };
        }

        static void EvalCompileTime(RightArguments right) {
            right.Compiler.AddCode(right.CodeLiteral.Content, right.CodeLiteral.Range.File.Filename, right.CodeLiteral.Range.Start);
        }

        class RightArguments : IRightArguments
        {
            public ICompiler Compiler;
            public IStringLiteral CodeLiteral;
        }
    }
}
