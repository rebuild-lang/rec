
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
                CompileTime = (left, right, result) => CompileTime((RightArguments) right)
            };
        }

        class RightArguments : IRightArguments
        {
            public ulong Value;
        }

        static void CompileTime(RightArguments right) {
            System.Console.WriteLine("Print: " + right.Value.ToString());
        }
    }
}