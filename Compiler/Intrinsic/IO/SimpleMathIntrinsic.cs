// ReSharper disable NotAccessedField.Local

using REC.Cpp;

#pragma warning disable 649

namespace REC.Intrinsic.IO
{
    interface ISimpleMath<T>
    {
        T Add(T l, T r);
        T Sub(T l, T r);
        // TODO: add more
    }

    class UlongMath : ISimpleMath<ulong>
    {
        public ulong Add(ulong l, ulong r) {
            return l + r;
        }

        public ulong Sub(ulong l, ulong r) {
            return l - r;
        }
    }

    static class SimpleMathIntrinsic<T, TA> where TA : ISimpleMath<T>, new()
    {
        static readonly TA Api = new TA();

        public static IFunctionIntrinsic[] Get() {
            return new IFunctionIntrinsic[] {
                new FunctionIntrinsic {
                    Name = "Assign",
                    RightArgumentsType = typeof(AssignArguments),
                    CompileTime = (left, right, result) => CompileTimeAssign((AssignArguments) right),
                    GenerateCpp = GenerateCppAssign
                },
                new FunctionIntrinsic {
                    Name = "Add",
                    RightArgumentsType = typeof(BinaryArguments),
                    ResultType = typeof(Result),
                    CompileTime = (left, right, result) => CompileTimeAdd((BinaryArguments) right, (Result) result),
                    GenerateCpp = GenerateCppAdd
                },
                new FunctionIntrinsic {
                    Name = "Sub",
                    RightArgumentsType = typeof(BinaryArguments),
                    ResultType = typeof(Result),
                    CompileTime = (left, right, result) => CompileTimeSub((BinaryArguments) right, (Result) result),
                    GenerateCpp = GenerateCppSub
                }
            };
        }

        static void CompileTimeAssign(AssignArguments args) {
            args.Left = args.Right;
        }

        static void GenerateCppAssign(ICppIntrinsic intrinsic) {
            intrinsic.Runtime.AddLine(
                $"{intrinsic.RightArgument(name: "Left")} = {intrinsic.RightArgument(name: "Right")};");
        }

        static void CompileTimeAdd(BinaryArguments args, Result res) {
            res.Value = Api.Add(args.Left, args.Right);
        }

        static void GenerateCppAdd(ICppIntrinsic intrinsic) {
            intrinsic.Runtime.AddLine(
                $"{intrinsic.ResultArgument(name: "Value")} = {intrinsic.RightArgument(name: "Left")} + {intrinsic.RightArgument(name: "Right")};");
        }

        static void CompileTimeSub(BinaryArguments args, Result res) {
            res.Value = Api.Sub(args.Left, args.Right);
        }

        static void GenerateCppSub(ICppIntrinsic intrinsic) {
            intrinsic.Runtime.AddLine(
                $"{intrinsic.ResultArgument(name: "Value")} = {intrinsic.RightArgument(name: "Left")} - {intrinsic.RightArgument(name: "Right")};");
        }

        class AssignArguments : IRightArguments
        {
            [ArgumentAssignable] public T Left;
            public T Right;
        }

        class BinaryArguments : IRightArguments
        {
            public T Left;
            public T Right;
        }

        class Result : IResultArguments
        {
            [ArgumentAssignable] public T Value;
        }
    }
}
