namespace REC.AST
{
    // Argument Declaration is part of a function declaration
    // Inside the function implementation block its treated like a variable
    // On calling the function it is used to associate argument values
    public interface IArgumentDeclaration : IVariableDeclaration
    {
        // if true this argument is an array or nested arguments, which captures without separation
        bool IsUnrolled { get; }
    }

    class ArgumentDeclaration : VariableDeclaration, IArgumentDeclaration
    {
        public bool IsUnrolled { get; set; }
    }
}
