using System.Collections.Generic;

namespace REC.AST
{
    public interface IFunctionDeclaration : IDeclaration
    {
        ICollection<IArgumentDeclaration> LeftArguments { get; }
        ICollection<IArgumentDeclaration> RightArguments { get; }
        ICollection<IArgumentDeclaration> Results { get; }

        IBlock Implementation { get; }
    }

    internal class FunctionDeclaration : Declaration, IFunctionDeclaration
    {
        public ICollection<IArgumentDeclaration> LeftArguments { get; set; }

        public ICollection<IArgumentDeclaration> RightArguments { get; set; }

        public ICollection<IArgumentDeclaration> Results { get; set; }


        public IBlock Implementation { get; set; }
    }
}