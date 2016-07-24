namespace REC.AST
{
    // Identifiers can be requested as a function argument
    public interface IIdentifierLiteral : ILiteral
    {
        string Content { get; }
    }

    internal class IdentifierLiteral : Literal, IIdentifierLiteral
    {
        public string Content { get; set; }
    }
}