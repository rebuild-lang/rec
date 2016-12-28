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
            return module.Scope.Identifiers[key: "type"] is IDeclaredEntry type
                && type.Declaration is IModuleDeclaration typeModule
                && typeModule.Scope.Identifiers[key: "size"] is IDeclaredEntry size
                && size.Declaration is IVariableDeclaration sizeVariable
                && sizeVariable.Type.Name == "u64";
            // TODO: read compile time value & check for != 0
        }

        public static ulong GetTypeSize(this IModuleDeclaration module) {
            return module.Scope.Identifiers[key: "type"] is IDeclaredEntry type
                && type.Declaration is IModuleDeclaration typeModule
                && typeModule.Scope.Identifiers[key: "size"] is IDeclaredEntry size
                && size.Declaration is IVariableDeclaration sizeVariable
                && sizeVariable.Type.Name == "u64"
                    ? 8u
                    : 0u;
            // TODO extract the real value
        }

        public static IFunctionEntry GetConstructor(this IModuleDeclaration module) {
            return module.Scope.Identifiers[key: "Construct"] as IFunctionEntry;
        }

        public static IFunctionEntry GetDestructor(this IModuleDeclaration module) {
            return module.Scope.Identifiers[key: "Destruct"] as IFunctionEntry;
        }

        public static IFunctionEntry GetImplicitFrom(this IModuleDeclaration module) {
            return module.Scope.Identifiers[key: "ImplicitFrom"] as IFunctionEntry;
        }

        public static FromLiteralFunc GetFromLiteral(this IModuleDeclaration module) {
            return (module as IntrinsicModuleDeclaration)?.FromLiteral;
        }

        public static ToNetTypeFunc GetToNetType(this IModuleDeclaration module) {
            return (module as IntrinsicModuleDeclaration)?.ToNetType;
        }

        public static FromNetTypeAction GetFromNetType(this IModuleDeclaration module) {
            return (module as IntrinsicModuleDeclaration)?.FromNetType;
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
