using System;
using REC.AST;

namespace REC.Intrinsic.Types
{
    static class U64Type
    {
        public static ITypeModuleIntrinsic Get() {
            return new TypeModuleIntrinsic {
                Name = "u64",
                TypeSize = 8,
                FromLiteral = FromLiteral,
                NetType = typeof(ulong),
                ToNetType = ToNetType,
                FromNetType = FromNetType
            };
        }

        static void FromNetType(dynamic net, byte[] bytes) {
            var value = (ulong) net;
            BitConverter.GetBytes(value).CopyTo(bytes, index: 0);
        }

        static object ToNetType(byte[] arg) {
            return BitConverter.ToUInt64(arg, startIndex: 0);
        }

        static LiteralConversion FromLiteral(byte[] dest, ILiteral literal) {
            return literal is INumberLiteral ? FromNumberLiteral(dest, (INumberLiteral) literal) : LiteralConversion.Failed;
        }

        static LiteralConversion FromNumberLiteral(byte[] dest, INumberLiteral numberLiteral) {
            if (numberLiteral.FitsUnsigned(byteCount: 8)) {
                Array.Copy(numberLiteral.ToUnsigned(byteCount: 8), dest, length: 8);
                return LiteralConversion.Ok;
            }
            return LiteralConversion.Failed;
        }
    }
}
