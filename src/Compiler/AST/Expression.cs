namespace REC.AST
{
    // Expression is the base class for all AST nodes
    public interface IExpression
    {
        // range of the entire expression
        // generated expressions use null
        TextFileRange Range { get; }
    }

    internal class Expression : IExpression
    {
        public TextFileRange Range { get; set; }
    }
}
