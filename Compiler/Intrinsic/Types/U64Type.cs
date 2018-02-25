using System;
using REC.AST;
using REC.Cpp;

namespace REC.Intrinsic.Types
{
    static class U64Type
    {
        public static ITypeModuleIntrinsic Get() {
            return new TypeModuleIntrinsic {
                Name = "u64",
                TypeSize = 8,
                FromExpression = FromExpression,
                NetType = typeof(ulong),
                ToNetType = ToNetType,
                FromNetType = FromNetType,
                GenerateCpp = GenerateCpp,
            };
        }

        static string GenerateCpp(byte[] bytes, ICppIntrinsic cpp) {
            var value = BitConverter.ToUInt64(bytes, startIndex: 0);
            return value.ToString();
        }

        static void FromNetType(dynamic net, byte[] bytes) {
            var value = (ulong)net;
            BitConverter.GetBytes(value).CopyTo(bytes, index: 0);
        }

        static object ToNetType(byte[] arg) {
            return BitConverter.ToUInt64(arg, startIndex: 0);
        }

        static ExpressionConversion FromExpression(byte[] dest, IExpression expression) {
            return expression is INumberLiteral numberLiteral ? FromNumberLiteral(dest, numberLiteral) : ExpressionConversion.Failed;
        }

        static ExpressionConversion FromNumberLiteral(byte[] dest, INumberLiteral numberLiteral) {
            if (numberLiteral.FitsUnsigned(byteCount: 8)) {
                Array.Copy(numberLiteral.ToUnsigned(byteCount: 8), dest, length: 8);
                return ExpressionConversion.Ok;
            }
            return ExpressionConversion.Failed;
        }
    }
}