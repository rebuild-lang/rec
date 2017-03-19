using System;
using REC.AST;
using REC.Tools;

namespace REC.Intrinsic.Types.API
{
    static class LiteralType<T> where T : class
    {
        static readonly HandleCache<T> Literals = new HandleCache<T>();

        public static ITypeModuleIntrinsic Get(string name) {
            return new TypeModuleIntrinsic {
                Name = name,
                TypeSize = 8,
                Construct = Construct,
                Destruct = Destruct,
                FromExpression = FromExpression,
                NetType = typeof(T),
                ToNetType = ToNetType,
                FromNetType = FromNetType,
                IsCompileTimeOnly = true,
            };
        }

        static void FromNetType(dynamic net, byte[] bytes) {
            var value = (T) net;
            var handle = Literals.GetHandle(value);
            Literals.AddRef(handle);
            BitConverter.GetBytes(handle).CopyTo(bytes, index: 0);
        }

        static object ToNetType(byte[] arg) {
            var handle = BitConverter.ToInt32(arg, startIndex: 0);
            return Literals.GetValue(handle);
        }

        static void Construct(byte[] data) {
            Array.Clear(data, index: 0, length: data.Length);
        }

        static void Destruct(byte[] data) {
            var handle = BitConverter.ToInt32(data, startIndex: 0);
            Literals.RemoveRef(handle);
        }

        static ExpressionConversion FromExpression(byte[] dest, IExpression expression) {
            if (!(expression is T blockLiteral)) return ExpressionConversion.Failed;
            var handle = Literals.GetHandle(blockLiteral);
            Literals.AddRef(handle);
            BitConverter.GetBytes(handle).CopyTo(dest, index: 0);
            return ExpressionConversion.Ok;
        }
    }
}
