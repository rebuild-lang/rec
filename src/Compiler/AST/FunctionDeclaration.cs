using System.Collections.Generic;

namespace REC.AST
{
    public interface IFunctionDeclaration : IDeclaration
    {
        IEnumerable<IArgumentDeclaration> LeftArguments { get; }
        IEnumerable<IArgumentDeclaration> RightArguments { get; }
        IEnumerable<IArgumentDeclaration> Results { get; }

        IBlock Implementation { get; }
    }

    internal class FunctionDeclaration : Declaration, IFunctionDeclaration
    {
        public IEnumerable<IArgumentDeclaration> LeftArguments { get; set; }

        public IEnumerable<IArgumentDeclaration> RightArguments { get; set; }

        public IEnumerable<IArgumentDeclaration> Results { get; set; }


        public IBlock Implementation { get; set; }
    }
}