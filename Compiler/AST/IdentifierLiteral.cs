using System;

namespace REC.AST
{
    [Flags]
    enum SeparatorNeighbor
    {
        None = 0, // 1+2            => + has no separators
        Left = 1, // +1, (+1), ;+1  => left separates
        Right = 2, // 1+, (1+), 1+; => right separates
        Both = 3, // 1 + 2          => both sides are separated
    }

    // Identifiers can be requested as a function argument
    public interface IIdentifierLiteral : ILiteral
    {
        string Content { get; }
        bool LeftSeparates { get; set; }
        bool RightSeparates { get; set; }
    }

    class IdentifierLiteral : Literal, IIdentifierLiteral
    {
        public string Content { get; set; }

        public bool LeftSeparates {
            get { return (_separatorNeighbor & SeparatorNeighbor.Left) != 0; }
            set {
                _separatorNeighbor = value
                    ? _separatorNeighbor | SeparatorNeighbor.Left
                    : _separatorNeighbor & ~SeparatorNeighbor.Left;
            }
        }

        public bool RightSeparates {
            get { return (_separatorNeighbor & SeparatorNeighbor.Right) != 0; }
            set
            {
                _separatorNeighbor = value
                    ? _separatorNeighbor | SeparatorNeighbor.Right
                    : _separatorNeighbor & ~SeparatorNeighbor.Right;
            }
        }

        public SeparatorNeighbor NeighborSeparator => _separatorNeighbor;

        SeparatorNeighbor _separatorNeighbor;
    }
}