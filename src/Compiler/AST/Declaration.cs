using REC.Tools;

namespace REC.AST
{
    // A declaration creates an entry in a scope
    // Uses are:
    // * TypedDeclarations (Define & Variables)
    // * FunctionDeclaration
    // * Modules
    public interface IDeclaration : IExpression, INamed
    {
        // Name might be null or empty! (allowed for unnamed modules)
    }

    abstract class Declaration : Expression, IDeclaration
    {
        public string Name { get; set; }
    }

    public interface ITypedDeclaration : IDeclaration
    {
        IModuleDeclaration Type { get; }
    }

    abstract class TypedDeclaration : Declaration, ITypedDeclaration
    {
        public IModuleDeclaration Type { get; set; }
    }
}