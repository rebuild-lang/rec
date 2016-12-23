using System;
using System.Collections.Generic;
using REC.AST;
using REC.Scanner;

namespace REC.Parser
{
    public static class TokenPreparation
    {
        static bool IsRightSeparator(this Token token) {
            switch (token) {
                case Token.WhiteSpaceSeperator:
                case Token.NewLineIndentation:
                case Token.Comment:
                case Token.CommaSeparator:
                case Token.SemicolonSeparator:
                case Token.SquareBracketClose:
                case Token.BracketClose:
                case Token.BlockEndIndentation:
                    return true;
                case Token.StringLiteral:
                case Token.NumberLiteral:
                case Token.IdentifierLiteral:
                case Token.OperatorLiteral:
                case Token.InvalidCharacter:
                case Token.SquareBracketOpen:
                case Token.BracketOpen:
                case Token.BlockStartIndentation:
                    return false;

                default:
                    throw new ArgumentOutOfRangeException(nameof(token), token, message: null);
            }
        }

        static bool IsLeftSeparator(this Token token) {
            switch (token) {
                case Token.WhiteSpaceSeperator: // [WS]tok
                case Token.NewLineIndentation: // [NL]tok
                case Token.Comment: // #* *#tok
                case Token.CommaSeparator: // ,tok
                case Token.SemicolonSeparator: // ;tok
                case Token.SquareBracketOpen: // [tok
                case Token.BracketOpen: // (tok
                case Token.BlockStartIndentation: // : tok
                    return true;
                case Token.StringLiteral: // ""+
                case Token.NumberLiteral: // 12+
                case Token.IdentifierLiteral: // ab+
                case Token.OperatorLiteral: // **+
                case Token.InvalidCharacter: // @@+
                case Token.SquareBracketClose: //  ]+
                case Token.BracketClose: //  )+
                case Token.BlockEndIndentation: // end+
                    return false;

                default:
                    throw new ArgumentOutOfRangeException(nameof(token), token, message: null);
            }
        }

        static void MarkRightSeparator(IIdentifierLiteral literal) {
            literal.RightSeparates = true;
        }

        static void MarkLeftSeparator(IIdentifierLiteral literal) {
            literal.LeftSeparates = true;
        }

        /**
         * Preparation step to ease the parsing of the token stream
         * 
         * filters out 
         * * invalid characters
         * * white spaces
         * * comments
         * * newline + indentations preceding comments
         * * two adjacent newline + indentations (basically multiple newlines)
         * * ":" between two indentations (error)
         * mutates
         * * ":" before indentation => block start
         * * "end" after indentation => block end
         * * identifiers separators around identifiers and operators
         * 
         * note:
         * * this code is independent of : being an operator or literal
         */

        public static IEnumerable<TokenData> Apply(IEnumerable<TokenData> input) {
            using (var it = input.GetEnumerator()) {
                do {
                    if (!it.MoveNext()) yield break;
                } while (it.Current.Type == Token.Comment
                    || it.Current.Type == Token.InvalidCharacter
                    || it.Current.Type == Token.WhiteSpaceSeperator); // skip to valid token
                var lastYieldType = Token.NewLineIndentation;
                var current = it.Current;
                if (current.Type != Token.NewLineIndentation) {
                    // first token has to be a newline so we have a proper indentation tracking going
                    yield return new TokenData {
                        Type = Token.NewLineIndentation,
                        Range = new TextFileRange {
                            End = current.Range?.Start,
                            File = current.Range?.File
                        }
                    };
                }
                var previous = current;
                if (previous.Type == Token.IdentifierLiteral || previous.Type == Token.OperatorLiteral)
                    MarkLeftSeparator((dynamic) previous.Data); // first identifier has space on the left
                while (it.MoveNext()) {
                    var previousOrSkippedType = current.Type;
                    current = it.Current;
                    if (previous.Type == Token.IdentifierLiteral || previous.Type == Token.OperatorLiteral) {
                        if (current.Type.IsRightSeparator()) MarkRightSeparator((dynamic) previous.Data);
                        if (previous.Range.Text == ":") {
                            // this might be a block start, keep it in buffer
                            while (current.Type == Token.WhiteSpaceSeperator
                                || current.Type == Token.Comment) {
                                if (!it.MoveNext()) {
                                    yield return previous; // report the regular token
                                    yield break; // finished
                                }
                                current = it.Current;
                            }
                        }
                    }
                    switch (current.Type) {
                        case Token.InvalidCharacter:
                        case Token.Comment:
                        case Token.WhiteSpaceSeperator:
                            continue;

                        case Token.NewLineIndentation:
                            if ((previous.Type == Token.IdentifierLiteral || previous.Type == Token.OperatorLiteral)
                                && previous.Range.Text == ":") {
                                if (lastYieldType == Token.NewLineIndentation
                                    || lastYieldType == Token.BlockStartIndentation) {
                                    // TODO: report as error
                                    // handling: ignore it
                                    continue;
                                }
                                current.Type = Token.BlockStartIndentation; // : + NewLine = BlockStart
                                previous = current;
                                continue;
                            }
                            if (previous.Type == Token.NewLineIndentation) continue; // skip double newlines
                            break;
                        case Token.OperatorLiteral:
                            if (previousOrSkippedType.IsLeftSeparator()) MarkLeftSeparator((dynamic) current.Data);
                            break;
                        case Token.IdentifierLiteral:
                            if (previous.Type == Token.NewLineIndentation && current.Range.Text == "end") {
                                previous.Type = Token.BlockEndIndentation; // NewLine + End = BlockEnd
                                current = previous; // setup for previousOrSkippedType
                                continue; // skip "end" token
                            }
                            if (previousOrSkippedType.IsLeftSeparator()) MarkLeftSeparator((dynamic) current.Data);
                            break;
                    }
                    lastYieldType = previous.Type;
                    yield return previous;
                    previous = current;
                }
                if (previous.Type == Token.IdentifierLiteral || previous.Type == Token.OperatorLiteral)
                    MarkRightSeparator((dynamic) previous.Data); // last identifier has space on the right
                if (previous.Type != Token.NewLineIndentation) yield return previous;
            }
        }
    }
}
