using REC.Tools;

namespace REC.AST
{
    // A declaration creates an instance in the parent scope
    // Uses are:
    // * Typed (Variables & Arguments)
    // * Functions
    // * Modules
    public interface IDeclaration : IExpression, INamed
    {
        // Name might be null or empty! (allowed for unnamed modules)

        bool IsCompileTimeOnly { get; }
    }

    abstract class Declaration : Expression, IDeclaration
    {
        public string Name { get; set; }
        public bool IsCompileTimeOnly { get; set; }
    }
}
