using System;
using REC.Cpp;

#pragma warning disable 649

namespace REC.Intrinsic.IO
{
    static class PrintIntrinsic
    {
        public static IFunctionIntrinsic Get() {
            return new FunctionIntrinsic {
                Name = ".Print",
                RightArgumentsType = typeof(RightArguments),
                CompileTime = (left, right, result) => CompileTime((RightArguments) right),
                GenerateCpp = GenerateCpp
            };
        }

        static void CompileTime(RightArguments right) {
            Console.WriteLine("Print: " + right.Value);
        }

        static void GenerateCpp(ICppIntrinsic intrinsic) {
            intrinsic.EnsureGlobal(concept: "stdio", func: () => "#include <stdio.h>");
            intrinsic.Runtime.AddLine($"printf(\"Print: %I64u\\n\", {intrinsic.RightArgument(name: "Value")});");
        }

        class RightArguments : IRightArguments
        {
            public ulong Value;
        }
    }
}
