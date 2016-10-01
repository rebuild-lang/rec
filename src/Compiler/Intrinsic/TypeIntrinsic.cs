using System;
using REC.AST;

namespace REC.Intrinsic
{
    using ConstructAction = Action<byte[]>;
    using DestructAction = Action<byte[]>;
    using ImplicitFromAction = Action<byte[] /*destination*/, ITypeModuleIntrinsic /*source_type*/, byte[] /*source_value*/>;
    using FromLiteralFunc = Func<byte[] /*destination*/, ILiteral, LiteralConversion>;
    using ToNetTypeFunc = Func<byte[] /*source*/, dynamic>;
    using FromNetTypeAction = Action<dynamic, byte[] /*destination*/>;


    [AttributeUsage(AttributeTargets.Class)]
    class CompileTimeOnly : Attribute
    { }

    public enum LiteralConversion
    {
        Ok,
        Failed, // no conversion possible
        TruncatedMajor, // input number > range (1000 => byte)
        TruncatedMinor, // not enough precision (1.5 => int)
        TruncatedRepresentation, // decimal float => binary float loss
    }

    public interface ITypeModuleIntrinsic : IModuleIntrinsic
    {
        ulong TypeSize { get; }

        ConstructAction Construct { get; }
        DestructAction Destruct { get; }

        ImplicitFromAction ImplicitFrom { get; }

        FromLiteralFunc FromLiteral { get; }

        Type NetType { get; }
        ToNetTypeFunc ToNetType { get; }
        FromNetTypeAction FromNetType { get; }
    }

    class TypeModuleIntrinsic : ModuleIntrinsic, ITypeModuleIntrinsic
    {
        public ulong TypeSize { get; set; }
        public ConstructAction Construct { get; set; }
        public DestructAction Destruct { get; set; }
        public ImplicitFromAction ImplicitFrom { get; set; }
        public FromLiteralFunc FromLiteral { get; set; }

        public Type NetType { get; set; }
        public ToNetTypeFunc ToNetType { get; set; }
        public FromNetTypeAction FromNetType { get; set; }
    }
}
