using System;
using REC.AST;
using REC.Tools;

namespace REC.Intrinsic.Types.API
{
    [CompileTimeOnly]
    static class BlockLiteralType
    {
        static readonly HandleCache<IBlockLiteral> BlockLiterals = new HandleCache<IBlockLiteral>();

        public static ITypeModuleIntrinsic Get() {
            return new TypeModuleIntrinsic {
                Name = "BlockLiteral",
                TypeSize = 8,
                Construct = Construct,
                Destruct = Destruct,
                FromLiteral = FromLiteral,
                NetType = typeof(IBlockLiteral),
                ToNetType = ToNetType,
                FromNetType = FromNetType
            };
        }

        static void FromNetType(dynamic net, byte[] bytes) {
            var value = (IBlockLiteral) net;
            var handle = BlockLiterals.GetHandle(value);
            BlockLiterals.AddRef(handle);
            BitConverter.GetBytes(handle).CopyTo(bytes, index: 0);
        }

        static object ToNetType(byte[] arg) {
            var handle = BitConverter.ToInt32(arg, startIndex: 0);
            return BlockLiterals.GetValue(handle);
        }

        static void Construct(byte[] data) {
            Array.Clear(data, index: 0, length: data.Length);
        }

        static void Destruct(byte[] data) {
            var handle = BitConverter.ToInt32(data, startIndex: 0);
            BlockLiterals.RemoveRef(handle);
        }

        static LiteralConversion FromLiteral(byte[] dest, ILiteral literal) {
            if (!(literal is IBlockLiteral blockLiteral)) return LiteralConversion.Failed;
            var handle = BlockLiterals.GetHandle(blockLiteral);
            BlockLiterals.AddRef(handle);
            BitConverter.GetBytes(handle).CopyTo(dest, index: 0);
            return LiteralConversion.Ok;
        }
    }
}
