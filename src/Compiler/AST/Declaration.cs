using System.Collections.Generic;

namespace REC.AST
{
    public interface IDeclaration : IExpression
    {
        string Name { get; }
    }

    internal class Declaration : Expression, IDeclaration
    {
        public string Name { get; set; }
    }
}