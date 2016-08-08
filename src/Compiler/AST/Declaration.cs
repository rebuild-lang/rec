using System.Collections.Generic;

namespace REC.AST
{
    public interface IDeclaration : IExpression
    {
        string Name { get; }
    }

    class Declaration : Expression, IDeclaration
    {
        public string Name { get; set; }
    }
}