using System.Collections.Generic;
using REC.Scanner;

namespace REC.AST
{
    public interface ITokenLine
    {
        IList<TokenData> Tokens { get; }
    }

    // block of code before it is parsed
    // used when a compile time function is invoked with a block argument
    public interface IBlockLiteral : ILiteral
    {
        IList<ITokenLine> Lines { get; }
    }

    class TokenLine : ITokenLine
    {
        public IList<TokenData> Tokens { get; } = new List<TokenData>();
    }

    class BlockLiteral : Declaration, IBlockLiteral
    {
        public IList<ITokenLine> Lines { get; } = new List<ITokenLine>();
    }
}
