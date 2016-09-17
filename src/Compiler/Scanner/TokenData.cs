namespace REC.Scanner
{
    public enum Token
    {
        WhiteSpaceSeperator,
        NewLineIndentation,
        BlockStartIndentation, // used later
        BlockEndIndentation, // used later
        Comment,
        CommaSeparator,
        SemicolonSeparator,
        SquareBracketOpen,
        SquareBracketClose,
        BracketOpen,
        BracketClose,
        StringLiteral,
        NumberLiteral,
        IdentifierLiteral,
        OperatorLiteral,
        InvalidCharacter,
    }

    public struct TokenData
    {
        public Token Type;
        public TextFileRange Range; // where the token comes from
        public object Data; // additional data for literals
        public object Error;
    }
}