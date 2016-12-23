using REC.Cpp;

#pragma warning disable 649

namespace REC.Intrinsic.IO
{
    static class PrintIntrinsic
    {
        public static IFunctionIntrinsic Get() {
            return new FunctionIntrinsic {
                Name = "Print",
                IsCompileTimeOnly = true,
                RightArgumentsType = typeof(RightArguments),
                CompileTime = (left, right, result) => CompileTime((RightArguments) right),
                GenerateCpp = GenerateCpp,
            };
        }

        class RightArguments : IRightArguments
        {
            public ulong Value;
        }

        static void CompileTime(RightArguments right) {
            System.Console.WriteLine("Print: " + right.Value.ToString());
        }

        static void GenerateCpp(ICppIntrinsic intrinsic) {
            intrinsic.EnsureGlobal("stdio", () => "#include <stdio.h>");
            intrinsic.Runtime.AddLine($"printf(\"Print: %I64u\\n\", {intrinsic.RightArgument("Value")});");
        }
    }
}