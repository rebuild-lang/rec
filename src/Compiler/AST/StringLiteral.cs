namespace REC.AST
{
    // raw string literal data
    public interface IStringLiteral : ILiteral
    {
        string Content { get; }
    }

    internal class StringLiteral : Literal, IStringLiteral
    {
        public string Content { get; set; }
    }
}