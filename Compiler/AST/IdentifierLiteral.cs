using System;

namespace REC.AST
{
    [Flags]
    enum SeparatorNeighbor
    {
        None = 0, // 1+2            => + has no separators
        Left = 1, // +1, (+1), ;+1  => left separates
        Right = 2, // 1+, (1+), 1+; => right separates
        Both = 3 // 1 + 2          => both sides are separated
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
        public SeparatorNeighbor NeighborSeparator { get; set; }

        public string Content { get; set; }

        public bool LeftSeparates {
            get { return (NeighborSeparator & SeparatorNeighbor.Left) != 0; }
            set {
                NeighborSeparator = value
                    ? NeighborSeparator | SeparatorNeighbor.Left
                    : NeighborSeparator & ~SeparatorNeighbor.Left;
            }
        }

        public bool RightSeparates {
            get { return (NeighborSeparator & SeparatorNeighbor.Right) != 0; }
            set {
                NeighborSeparator = value
                    ? NeighborSeparator | SeparatorNeighbor.Right
                    : NeighborSeparator & ~SeparatorNeighbor.Right;
            }
        }
    }
}
