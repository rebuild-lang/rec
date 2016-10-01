using System;

namespace REC.Intrinsic
{
    // use this intrinsic to mark arguments to be unrolled
    // allowed for Collection<> and Dictionary<String, T> types
    [AttributeUsage(AttributeTargets.Field)]
    class ArgumentUnrolled : Attribute
    { }

    // mark classes for use as left arguments
    public interface ILeftArguments
    { }

    public interface IRightArguments
    { }

    public interface IResultArguments
    { }

    public interface IFunctionIntrinsic : IIntrinsic
    {
        bool IsCompileTimeOnly { get; }

        Type ResultType { get; }

        Type LeftArgumentsType { get; }

        Type RightArgumentsType { get; }

        Action<ILeftArguments, IRightArguments, IResultArguments> CompileTime { get; }
    }

    class FunctionIntrinsic : Intrinsic, IFunctionIntrinsic
    {
        public bool IsCompileTimeOnly { get; set; }
        public Type ResultType { get; set; }
        public Type LeftArgumentsType { get; set; }
        public Type RightArgumentsType { get; set; }
        public Action<ILeftArguments, IRightArguments, IResultArguments> CompileTime { get; set; }
    }
}
