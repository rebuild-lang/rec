using System;
using REC.AST;
using REC.Tools;

namespace REC.Intrinsic.Types.API
{
    [CompileTimeOnly]
    static class NumberLiteralType
    {
        static readonly HandleCache<INumberLiteral> NumberLiterals = new HandleCache<INumberLiteral>();

        public static ITypeModuleIntrinsic Get() {
            return new TypeModuleIntrinsic {
                Name = "NumberLiteralType",
                TypeSize = 8,
                Construct = Construct,
                Destruct = Destruct,
                FromLiteral = FromLiteral,

                NetType = typeof(INumberLiteral),
                ToNetType = ToNetType,
                FromNetType = FromNetType,
            };
        }

        static void FromNetType(dynamic net, byte[] bytes) {
            var value = (INumberLiteral)net;
            var handle = NumberLiterals.GetHandle(value);
            NumberLiterals.AddRef(handle);
            BitConverter.GetBytes(handle).CopyTo(bytes, index: 0);
        }

        static object ToNetType(byte[] arg) {
            var handle = BitConverter.ToInt32(arg, startIndex: 0);
            return NumberLiterals.GetValue(handle);
        }

        static void Construct(byte[] data) {
            Array.Clear(data, index: 0, length: data.Length);
        }
        static void Destruct(byte[] data) {
            var handle = BitConverter.ToInt32(data, startIndex: 0);
            NumberLiterals.RemoveRef(handle);
        }

        static LiteralConversion FromLiteral(byte[] dest, ILiteral literal) {
            var numberLiteral = literal as INumberLiteral;
            if (null == numberLiteral) return LiteralConversion.Failed;
            var handle = NumberLiterals.GetHandle(numberLiteral);
            NumberLiterals.AddRef(handle);
            BitConverter.GetBytes(handle).CopyTo(dest, index: 0);
            return LiteralConversion.Ok;
        }
    }
}
