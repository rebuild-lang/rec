using System;
using System.Collections.Generic;
using REC.Intrinsic;
using REC.Parser;
using REC.Scope;

namespace REC.AST
{
    using FromLiteralFunc = Func<byte[] /*destination*/, ILiteral, LiteralConversion>;
    using ToNetTypeFunc = Func<byte[] /*source*/, dynamic>;
    using FromNetTypeAction = Action<dynamic, byte[] /*destination*/>;

    // Modules declare a named identifierScope
    // Usages:
    // * create compile time name spaces
    // * represent a runtime type
    //   - define a compile time 'type.size'
    //   - optionally define runtime construct & destruct methods
    // * extend the module with the compile time 'module.define' function
    // * parse a token block in the context of the module identifierScope with the compile time 'module.with' function
    public interface IModuleDeclaration : IDeclaration, IExpressionBlock
    {
        IScope Scope { get; }
    }

    public interface IType
    {
        IModuleDeclaration Module { get; }
        ulong Size { get; }

        void Construct(byte[] data);

        void Destruct(byte[] data);
    }

    public static class ModuleDeclarationExt
    {
        public static bool IsType(this IModuleDeclaration module) {
            var typeSize = (((module.Scope.Identifiers["type"] as IDeclaredEntry)?.Declaration as IModuleDeclaration)?.Scope?.Identifiers?["size"] as IDeclaredEntry);
            if (null == typeSize) return false;
            // TODO: read compile time value & check for != 0
            return true;
        }

        public static ulong GetTypeSize(this IModuleDeclaration module) {
            var entry = (((module.Scope.Identifiers["type"] as IDeclaredEntry)?.Declaration as IModuleDeclaration)?.Scope?.Identifiers?["size"] as IDeclaredEntry);
            // TODO extract the real value
            return entry != null ? 8u : 0u;
        }

        public static IFuntionEntry GetConstructor(this IModuleDeclaration module) {
            return module.Scope.Identifiers["Construct"] as IFuntionEntry;
        }
        public static IFuntionEntry GetDestructor(this IModuleDeclaration module) {
            return module.Scope.Identifiers["Destruct"] as IFuntionEntry;
        }

        public static IFuntionEntry GetImplicitFrom(this IModuleDeclaration module) {
            return module.Scope.Identifiers["ImplicitFrom"] as IFuntionEntry;
        }

        public static FromLiteralFunc GetFromLiteral(this IModuleDeclaration module) {
            return (module as IntrinsicModuleDeclaration)?.FromLiteral;
        }

        public static ToNetTypeFunc GetToNetType(this IModuleDeclaration module) {
            return (module as IntrinsicModuleDeclaration)?.ToNetType;
        }

    }

    class ModuleDeclaration : Declaration, IModuleDeclaration
    {
        public IList<IExpression> Expressions { get; set; } = new List<IExpression>();
        public IScope Scope { get; set; }
    }

    class IntrinsicModuleDeclaration : ModuleDeclaration
    {
        public FromLiteralFunc FromLiteral { get; set; }
        public Type NetType { get; set; }
        public ToNetTypeFunc ToNetType { get; set; }
        public FromNetTypeAction FromNetType { get; set; }
    }
}