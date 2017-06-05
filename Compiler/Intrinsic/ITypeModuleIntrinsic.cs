using System;
using REC.AST;
using REC.Cpp;

namespace REC.Intrinsic
{
    public interface ITypeModuleIntrinsic : IModuleIntrinsic
    {
        bool IsCompileTimeOnly { get; }
        ulong TypeSize { get; }

        Action<byte[]> Construct { get; }
        Action<byte[]> Destruct { get; }

        Action<byte[], ITypeModuleIntrinsic, byte[]> ImplicitFrom { get; }

        Func<byte[], IExpression, ExpressionConversion> FromExpression { get; }

        Func<byte[], ICppIntrinsic, string> GenerateCpp { get; }

        Type NetType { get; }
        Func<byte[], dynamic> ToNetType { get; }
        Action<dynamic, byte[]> FromNetType { get; }
    }
}