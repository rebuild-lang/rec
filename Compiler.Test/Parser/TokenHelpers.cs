using REC.AST;
using REC.Scanner;

namespace REC.Tests.Parser
{
    public class TokenHelpers
    {
        internal static TokenData Id(string text) {
            return new TokenData {
                Type = Token.IdentifierLiteral,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                },
                Data = new IdentifierLiteral {
                    Content = text
                }
            };
        }

        internal static TokenData Op(string text) {
            return new TokenData {
                Type = Token.OperatorLiteral,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                },
                Data = new IdentifierLiteral {
                    Content = text
                }
            };
        }

        internal static TokenData StringLiteral(string text) {
            return new TokenData {
                Type = Token.StringLiteral,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                }
            };
        }

        internal static TokenData NewLineIndentation(int column) {
            return new TokenData {
                Type = Token.NewLineIndentation,
                Range = new TextFileRange {
                    End = new TextPosition {Column = column}
                }
            };
        }

        internal static TokenData BlockStart(int column, IBlockLiteral block = null) {
            return new TokenData {
                Type = Token.BlockStartIndentation,
                Range = new TextFileRange {
                    End = new TextPosition {Column = column}
                },
                Data = block
            };
        }

        internal static TokenData BlockEnd(int column) {
            return new TokenData {
                Type = Token.BlockEndIndentation,
                Range = new TextFileRange {
                    End = new TextPosition {Column = column}
                }
            };
        }

        internal static TokenData BracketOpen() {
            const string text = "(";
            return new TokenData {
                Type = Token.BracketOpen,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                }
            };
        }

        internal static TokenData BracketClose() {
            const string text = ")";
            return new TokenData {
                Type = Token.BracketClose,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                }
            };
        }

        internal static TokenData NumberLit(string text) {
            return new TokenData {
                Type = Token.NumberLiteral,
                Range = new TextFileRange {
                    File = new TextFile {Content = text},
                    End = new TextPosition {Column = text.Length, Index = text.Length}
                },
                Data = new NumberLiteral {
                    Radix = 10,
                    IntegerPart = text
                }
            };
        }
    }
}
