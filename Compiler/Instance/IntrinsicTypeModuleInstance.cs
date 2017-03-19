﻿using System;
using REC.AST;
using REC.Intrinsic;

namespace REC.Instance
{
    using FromExpressionFunc = Func<byte[] /*destination*/, IExpression, ExpressionConversion>;
    using ToNetTypeFunc = Func<byte[] /*source*/, dynamic>;
    using FromNetTypeAction = Action<dynamic, byte[] /*destination*/>;

    interface IIntrinsicTypeModuleInstance : IModuleInstance
    {
        FromExpressionFunc FromExpression { get; }
        Type NetType { get; }
        ToNetTypeFunc ToNetType { get; }
        FromNetTypeAction FromNetType { get; }
    }


    class IntrinsicTypeModuleInstance : ModuleInstance, IIntrinsicTypeModuleInstance
    {
        public FromExpressionFunc FromExpression { get; }
        public Type NetType { get; }
        public ToNetTypeFunc ToNetType { get; }
        public FromNetTypeAction FromNetType { get; }

        public IntrinsicTypeModuleInstance(ITypeModuleIntrinsic typeIntrinsic) : base(typeIntrinsic.Name) {
            IsCompileTimeOnly = typeIntrinsic.IsCompileTimeOnly;
            FromExpression = typeIntrinsic.FromExpression;
            NetType = typeIntrinsic.NetType;
            ToNetType = typeIntrinsic.ToNetType;
            FromNetType = typeIntrinsic.FromNetType;
        }
    }

    static class ModuleInstanceIntrinsicExt
    {
        public static FromExpressionFunc GetFromExpression(this IModuleInstance module) {
            return (module as IIntrinsicTypeModuleInstance)?.FromExpression;
        }

        public static ToNetTypeFunc GetToNetType(this IModuleInstance module) {
            return (module as IIntrinsicTypeModuleInstance)?.ToNetType;
        }

        public static FromNetTypeAction GetFromNetType(this IModuleInstance module) {
            return (module as IIntrinsicTypeModuleInstance)?.FromNetType;
        }
    }
}
