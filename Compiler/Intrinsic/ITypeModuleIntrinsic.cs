using System;
using REC.AST;

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

        Type NetType { get; }
        Func<byte[], dynamic> ToNetType { get; }
        Action<dynamic, byte[]> FromNetType { get; }
    }
}