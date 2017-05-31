namespace REC.AST
{
    // Compiler Phase
    public interface IPhaseDeclaration : IDeclaration
    {
        IExpressionBlock Block { get; }
    }

    class PhaseDeclaration : Declaration, IPhaseDeclaration
    {
        public IExpressionBlock Block { get; set; } = new ExpressionBlock();
    }
}
