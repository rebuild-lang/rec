using System;
using REC.Cpp;
using REC.Parser;

namespace REC.Intrinsic
{
    // use this intrinsic to mark arguments to be unrolled
    // allowed for Collection<> and Dictionary<String, T> types

    // mark classes for use as left arguments

    class FunctionIntrinsic : Intrinsic, IFunctionIntrinsic
    {
        public bool IsCompileTimeOnly { get; set; }
        public Type ResultType { get; set; }
        public Type LeftArgumentsType { get; set; }
        public Type RightArgumentsType { get; set; }
        public Action<ILeftArguments, IRightArguments, IResultArguments> CompileTime { get; set; }
        public Action<ILeftArguments, IRightArguments, IResultArguments, IContext> CompileTimeApi { get; set; }
        public Action<ICppIntrinsic> GenerateCpp { get; set; }
    }
}
