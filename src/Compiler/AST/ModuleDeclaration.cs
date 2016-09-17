using System.Collections.Generic;
using REC.Scope;

namespace REC.AST
{
    // Modules declare a named scope
    // Usages:
    // * create compile time name spaces
    // * represent a runtime type
    //   - define a compile time 'type.size'
    //   - optionally define runtime construct & destruct methods
    // * extend the module with the compile time 'module.define' function
    // * parse a token block in the context of the module scope with the compile time 'module.with' function
    public interface IModuleDeclaration : IDeclaration, IExpressionBlock
    {
        IScope Scope { get; }
    }

    public static class ModuleDeclarationExt
    {
        public static bool IsType(this IModuleDeclaration module) {
            var type_size = (((module.Scope["type"] as IDeclaredEntry)?.Declaration as IModuleDeclaration)?.Scope?["size"] as IDeclaredEntry);
            if (null == type_size) return false;
            // TODO: read compile time value & check for != 0
            return true;
        }
    }

    internal class ModuleDeclaration : Declaration, IModuleDeclaration
    {
        public ICollection<IExpression> Expressions { get; } = new List<IExpression>();
        public IScope Scope { get; set; }
    }
}