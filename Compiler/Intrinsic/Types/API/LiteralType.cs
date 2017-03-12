using System;
using REC.AST;
using REC.Tools;

namespace REC.Intrinsic.Types.API
{
    [CompileTimeOnly]
    static class LiteralType<T>
    {
        static readonly HandleCache<T> Literals = new HandleCache<T>();

        public static ITypeModuleIntrinsic Get(string name) {
            return new TypeModuleIntrinsic {
                Name = name,
                TypeSize = 8,
                Construct = Construct,
                Destruct = Destruct,
                FromLiteral = FromLiteral,
                NetType = typeof(T),
                ToNetType = ToNetType,
                FromNetType = FromNetType
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

        static LiteralConversion FromLiteral(byte[] dest, ILiteral literal) {
            if (!(literal is T blockLiteral)) return LiteralConversion.Failed;
            var handle = Literals.GetHandle(blockLiteral);
            Literals.AddRef(handle);
            BitConverter.GetBytes(handle).CopyTo(dest, index: 0);
            return LiteralConversion.Ok;
        }
    }
}
