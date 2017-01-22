using System;
using REC.AST;

namespace REC.Intrinsic
{
    public interface ITypeModuleIntrinsic : IModuleIntrinsic
    {
        ulong TypeSize { get; }

        Action<byte[]> Construct { get; }
        Action<byte[]> Destruct { get; }

        Action<byte[], ITypeModuleIntrinsic, byte[]> ImplicitFrom { get; }

        Func<byte[], ILiteral, LiteralConversion> FromLiteral { get; }

        Type NetType { get; }
        Func<byte[], dynamic> ToNetType { get; }
        Action<dynamic, byte[]> FromNetType { get; }
    }
}