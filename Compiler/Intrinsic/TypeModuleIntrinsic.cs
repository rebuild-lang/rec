using System;
using REC.AST;

namespace REC.Intrinsic
{
    class TypeModuleIntrinsic : ModuleIntrinsic, ITypeModuleIntrinsic
    {
        public bool IsCompileTimeOnly { get; set; }
        public ulong TypeSize { get; set; }
        public Action<byte[]> Construct { get; set; } = DefaultConstruct;

        public Action<byte[]> Destruct { get; set; }
        public Action<byte[], ITypeModuleIntrinsic, byte[]> ImplicitFrom { get; set; }
        public Func<byte[], IExpression, ExpressionConversion> FromExpression { get; set; }

        public Type NetType { get; set; }
        public Func<byte[], dynamic> ToNetType { get; set; }
        public Action<dynamic, byte[]> FromNetType { get; set; }

        static void DefaultConstruct(byte[] data) {
            Array.Clear(data, index: 0, length: data.Length);
        }
    }
}