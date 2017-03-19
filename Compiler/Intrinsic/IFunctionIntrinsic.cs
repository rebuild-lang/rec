using System;
using REC.Cpp;
using REC.Parser;

namespace REC.Intrinsic
{
    public interface IFunctionIntrinsic : IIntrinsic
    {
        bool IsCompileTimeOnly { get; }

        Type ResultType { get; }

        Type LeftArgumentsType { get; }

        Type RightArgumentsType { get; }

        Action<ILeftArguments, IRightArguments, IResultArguments> CompileTime { get; }
        Action<ILeftArguments, IRightArguments, IResultArguments, IContext> CompileTimeApi { get; }

        Action<ICppIntrinsic> GenerateCpp { get; }
    }
}