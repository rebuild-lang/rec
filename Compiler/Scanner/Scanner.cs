using System.Collections.Generic;
using System.Diagnostics;

namespace REC.Scanner
{
    public static class Scanner
    {
        public static IEnumerable<TokenData> ScanFile(TextFile file) {
            var input = new TextInputRange {File = file};
            while (true) {
                input.Collapse();
                var chr = input.EndChar;
                // ReSharper disable once SwitchStatementMissingSomeCases
                switch (chr) {
                    case '\0':
                        yield break;
                    case ' ':
                    case '\t':
                        yield return ScanWhitespaces(input);
                        continue;
                    case '\n':
                    case '\r':
                        yield return ScanNewLine(input);
                        continue;
                    case '#':
                        yield return ScanComment(input);
                        continue;
                    case ',':
                        yield return ScanSingleChar(input, Token.CommaSeparator);
                        continue;
                    case ';':
                        yield return ScanSingleChar(input, Token.SemicolonSeparator);
                        continue;
                    case '[':
                        yield return ScanSingleChar(input, Token.SquareBracketOpen);
                        continue;
                    case ']':
                        yield return ScanSingleChar(input, Token.SquareBracketClose);
                        continue;
                    case '(':
                        yield return ScanSingleChar(input, Token.BracketOpen);
                        continue;
                    case ')':
                        yield return ScanSingleChar(input, Token.BracketClose);
                        continue;
                    case '"':
                        yield return ScanStringLiteral(input);
                        continue;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        yield return ScanNumberLiteral(input);
                        continue;
                }
                var identifierLiteral = IdentifierScanner.Scan(input);
                if (identifierLiteral != null) {
                    yield return new TokenData {Range = identifierLiteral.Range, Type = Token.IdentifierLiteral, Data = identifierLiteral};
                    continue;
                }
                var operatorLiteral = OperatorScanner.Scan(input);
                if (operatorLiteral != null) {
                    yield return new TokenData {Range = operatorLiteral.Range, Type = Token.OperatorLiteral, Data = operatorLiteral};
                    continue;
                }
                input.Extend();
                yield return new TokenData {Range = input.Clone(), Type = Token.InvalidCharacter};
            }
        }

        static TokenData ScanWhitespaces(TextInputRange input) {
            input.Extend();
            input.ExtendWhitespaces();
            return new TokenData {Range = input.Clone(), Type = Token.WhiteSpaceSeperator};
        }

        static TokenData ScanNewLine(TextInputRange input) {
            input.NewLine();
            input.ExtendWhitespaces();
            return new TokenData {Range = input.Clone(), Type = Token.NewLineIndentation};
        }

        static TokenData ScanComment(TextInputRange input) {
            var comment = CommentScanner.Scan(input);
            Debug.Assert(comment);
            return new TokenData {Range = input.Clone(), Type = Token.Comment};
        }

        static TokenData ScanSingleChar(TextInputRange input, Token token) {
            input.Extend();
            return new TokenData {Range = input.Clone(), Type = token};
        }

        static TokenData ScanStringLiteral(TextInputRange input) {
            var literal = StringLiteralScanner.Scan(input);
            Debug.Assert(literal != null); // TODO: Create error token
            return new TokenData {Range = literal.Range, Type = Token.StringLiteral, Data = literal};
        }

        static TokenData ScanNumberLiteral(TextInputRange input) {
            var literal = NumberLiteralScanner.Scan(input);
            Debug.Assert(literal != null); // TODO: Create error token
            return new TokenData {Range = literal.Range, Type = Token.NumberLiteral, Data = literal};
        }
    }
}
