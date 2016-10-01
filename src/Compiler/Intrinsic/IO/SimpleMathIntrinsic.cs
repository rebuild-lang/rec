﻿// ReSharper disable NotAccessedField.Local
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
                    Name = "Add",
                    RightArgumentsType = typeof(BinaryArguments),
                    ResultType = typeof(Result),
                    CompileTime = (left, right, result) => CompileTimeAdd((BinaryArguments) right, (Result) result)
                },
                new FunctionIntrinsic {
                    Name = "Sub",
                    RightArgumentsType = typeof(BinaryArguments),
                    ResultType = typeof(Result),
                    CompileTime = (left, right, result) => CompileTimeSub((BinaryArguments) right, (Result) result)
                },
            };
        }

        class BinaryArguments : IRightArguments
        {
            public T Left;
            public T Right;
        }

        class Result : IResultArguments
        {
            public T Value;
        }

        static void CompileTimeAdd(BinaryArguments args, Result res) {
            res.Value = Api.Add(args.Left, args.Right);
        }

        static void CompileTimeSub(BinaryArguments args, Result res) {
            res.Value = Api.Sub(args.Left, args.Right);
        }
    }
}