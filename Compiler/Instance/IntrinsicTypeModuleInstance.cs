using System;
using REC.AST;
using REC.Intrinsic;

namespace REC.Instance
{
    using FromLiteralFunc = Func<byte[] /*destination*/, ILiteral, LiteralConversion>;
    using ToNetTypeFunc = Func<byte[] /*source*/, dynamic>;
    using FromNetTypeAction = Action<dynamic, byte[] /*destination*/>;

    interface IIntrinsicTypeModuleInstance : IModuleInstance
    {
        FromLiteralFunc FromLiteral { get; }
        Type NetType { get; }
        ToNetTypeFunc ToNetType { get; }
        FromNetTypeAction FromNetType { get; }
    }


    class IntrinsicTypeModuleInstance : ModuleInstance, IIntrinsicTypeModuleInstance
    {
        public FromLiteralFunc FromLiteral { get; }
        public Type NetType { get; }
        public ToNetTypeFunc ToNetType { get; }
        public FromNetTypeAction FromNetType { get; }

        public IntrinsicTypeModuleInstance(ITypeModuleIntrinsic typeIntrinsic) : base(typeIntrinsic.Name) {
            FromLiteral = typeIntrinsic.FromLiteral;
            NetType = typeIntrinsic.NetType;
            ToNetType = typeIntrinsic.ToNetType;
            FromNetType = typeIntrinsic.FromNetType;
        }
    }

    static class ModuleInstanceIntrinsicExt
    {
        public static FromLiteralFunc GetFromLiteral(this IModuleInstance module) {
            return (module as IIntrinsicTypeModuleInstance)?.FromLiteral;
        }

        public static ToNetTypeFunc GetToNetType(this IModuleInstance module) {
            return (module as IIntrinsicTypeModuleInstance)?.ToNetType;
        }

        public static FromNetTypeAction GetFromNetType(this IModuleInstance module) {
            return (module as IIntrinsicTypeModuleInstance)?.FromNetType;
        }
    }
}
