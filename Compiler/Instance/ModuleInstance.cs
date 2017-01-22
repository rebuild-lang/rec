using REC.AST;
using REC.Scope;
using System.Collections.Generic;

namespace REC.Instance
{
    public interface IModuleInstance : IInstance
    {
        ILocalIdentifierScope Identifiers { get; }

        //ILocalValueScope Values { get; }

        IList<IModuleDeclaration> Declarations { get; }
    }

    class ModuleInstance : AbstractInstance, IModuleInstance
    {
        public override string Name { get; }
        public IList<IModuleDeclaration> Declarations { get; } = new List<IModuleDeclaration>();
        public ILocalIdentifierScope Identifiers { get; } = new LocalIdentifierScope();
        //public ILocalValueScope Values { get; } = new LocalValueScope();

        public ModuleInstance(string name) {
            Name = name;
        }

        public ModuleInstance(IModuleDeclaration decl) {
            Name = decl.Name;
            Declarations.Add(decl);
        }
    }


    //public interface IType
    //{
    //    IModuleDeclaration Module { get; }

    //    ulong Size { get; }

    //    void Construct(byte[] data);

    //    void Destruct(byte[] data);
    //}

    public static class ModuleInstanceTypeExt
    {
        public static bool IsType(this IModuleInstance module) {
            return module.Identifiers[key: "type"] is IModuleInstance typeModule
                && typeModule.Identifiers[key: "size"] is IVariableInstance sizeVariable
                && sizeVariable.Type.Name == "u64";
            // TODO: read compile time value & check for != 0
        }

        public static ulong GetTypeSize(this IModuleInstance module) {
            return module.Identifiers[key: "type"] is IModuleInstance typeModule
                && typeModule.Identifiers[key: "size"] is IVariableInstance sizeVariable
                && sizeVariable.Type.Name == "u64"
                    ? 8u
                    : 0u;
            // TODO extract the real value
        }

        public static IFunctionInstance GetConstructor(this IModuleInstance module) {
            return module.Identifiers[key: "Construct"] as IFunctionInstance;
        }

        public static IFunctionInstance GetDestructor(this IModuleInstance module) {
            return module.Identifiers[key: "Destruct"] as IFunctionInstance;
        }

        public static IFunctionInstance GetImplicitFrom(this IModuleInstance module) {
            return module.Identifiers[key: "ImplicitFrom"] as IFunctionInstance;
        }
    }
}
