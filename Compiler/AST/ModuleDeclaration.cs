namespace REC.AST
{
    // Modules declare a named IdentifierScope
    // Usages:
    // * create compile time name spaces
    // * represent a runtime type
    //   - define a compile time 'type.size'
    //   - optionally define runtime construct & destruct methods
    // * extend the module with the compile time 'module.define' function
    // * parse a token block in the context of the module identifierScope with the compile time 'module.with' function
    public interface IModuleDeclaration : IDeclaration
    {
        IExpressionBlock Block { get; }
    }

    class ModuleDeclaration : Declaration, IModuleDeclaration
    {
        public IExpressionBlock Block { get; set; } = new ExpressionBlock();
    }
}
