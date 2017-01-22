using System;
using REC.Cpp;

namespace REC.Intrinsic
{
    public interface IFunctionIntrinsic : IIntrinsic
    {
        bool IsCompileTimeOnly { get; }

        Type ResultType { get; }

        Type LeftArgumentsType { get; }

        Type RightArgumentsType { get; }

        Action<ILeftArguments, IRightArguments, IResultArguments> CompileTime { get; }

        Action<ICppIntrinsic> GenerateCpp { get; }
    }
}