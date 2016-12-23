using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using REC.AST;
using REC.Parser;
using REC.Scanner;

namespace REC.Tests.Parser
{
    [TestFixture()]
    public class TokenPreparationTests
    {
        [TestCase(arg1: new object[] {
            Token.Comment,
            Token.NewLineIndentation, "",
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral
        }, TestName = "Filter out starting comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, Token.Comment,
            Token.NewLineIndentation, ""
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Filter out starting indented comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, Token.Comment, Token.WhiteSpaceSeperator, Token.Comment,
            Token.NewLineIndentation, ""
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Filter out comment whitespace comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "", Token.Comment,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Filter out final comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "", Token.WhiteSpaceSeperator,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Filter out final whitespace")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation,
            Token.NewLineIndentation, "",
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Filter multiple newlines")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation,
            Token.NewLineIndentation,
            Token.NewLineIndentation, "",
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Filter even more newlines")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "", Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral,
        }, TestName = "Filter out final newline")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "end", Token.NewLineIndentation,
        }, arg2: new[] {
            Token.BlockEndIndentation,
        }, TestName = "Mutate block end")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "begin", ":", Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral, Token.BlockStartIndentation,
        }, TestName = "Mutate identifier block start")]

        [TestCase(arg1: new object[] {
            "begin", ":", Token.WhiteSpaceSeperator, Token.Comment, Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral, Token.BlockStartIndentation,
        }, TestName = "Mutate identifier block start with comment")]

        [TestCase(arg1: new object[] {
            Token.NewLineIndentation, "begin", ':', Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral, Token.BlockStartIndentation,
        }, TestName = "Mutate operator block start")]

        [TestCase(arg1: new object[] {
            "begin", ':', Token.WhiteSpaceSeperator, Token.Comment, Token.NewLineIndentation,
        }, arg2: new[] {
            Token.NewLineIndentation, Token.IdentifierLiteral, Token.BlockStartIndentation,
        }, TestName = "Mutate operator block start with comment")]

        public void Filter(IEnumerable<dynamic> inputTokens, IEnumerable<Token> expectedTokens) {
            var input = inputTokens.Select(
                t => {
                    var s = t as string;
                    if (s != null) {
                        return new TokenData {
                            Type = Token.IdentifierLiteral,
                            Range = new TextFileRange { File = new TextFile { Content = s }, End = new TextPosition { Index = s.Length } },
                            Data = new IdentifierLiteral { Content = s }
                        };
                    }
                    var c = t as char?;
                    if (c != null) {
                        return new TokenData {
                            Type = Token.OperatorLiteral,
                            Range = new TextFileRange { File = new TextFile { Content = ""+c }, End = new TextPosition { Index = 1 } },
                            Data = new IdentifierLiteral { Content = ""+c }
                        };
                    }
                    return new TokenData { Type = t };
                });

            var prepared = TokenPreparation.Apply(input).ToArray();

            Assert.AreEqual(expected: expectedTokens, actual: prepared.Select(t => t.Type));
        }

        [Flags]
        public enum TestNeighor
        {
            None = SeparatorNeighbor.None,
            Left = SeparatorNeighbor.Left,
            Right = SeparatorNeighbor.Right,
            Both = SeparatorNeighbor.Both,
        }

        [TestCase(arg1: new object[] {
            Token.WhiteSpaceSeperator, "left", "none", "right", Token.WhiteSpaceSeperator, "both", Token.WhiteSpaceSeperator
        }, arg2: new[] {
            TestNeighor.Left, TestNeighor.None, TestNeighor.Right, TestNeighor.Both,
        }, TestName = "With white spaces")]

        [TestCase(arg1: new object[] {
            "left", "right"
        }, arg2: new[] {
            TestNeighor.Left, TestNeighor.Right
        }, TestName = "Border Cases")]

        [TestCase(arg1: new object[] {
            Token.BracketOpen, "left", "right", Token.BracketClose, "none", Token.BracketOpen
        }, arg2: new[] {
            TestNeighor.Left, TestNeighor.Right, TestNeighor.None,
        }, TestName = "Brackets")]


        [TestCase(arg1: new object[] {
            Token.WhiteSpaceSeperator, "a", Token.CommaSeparator, "b", Token.WhiteSpaceSeperator,
        }, arg2: new[] {
            TestNeighor.Both, TestNeighor.Both
        }, TestName = "Comma")]

        [TestCase(arg1: new object[] {
            Token.WhiteSpaceSeperator, "a", Token.SemicolonSeparator, "b", Token.WhiteSpaceSeperator,
        }, arg2: new[] {
            TestNeighor.Both, TestNeighor.Both
        }, TestName = "Semicolon")]
        public void IdentifierSeparators(dynamic[] inputTokens, IEnumerable<TestNeighor> neighbors) {
            var inputIdentifier = inputTokens.Select(
                t => {
                    var s = t as string;
                    if (s != null) {
                        return new TokenData {
                            Type = Token.IdentifierLiteral,
                            Range = new TextFileRange { File = new TextFile { Content = s }, End = new TextPosition { Index = s.Length } },
                            Data = new IdentifierLiteral { Content = s }
                        };
                    }
                    return new TokenData { Type = t };
                });

            var preparedIdentifier = TokenPreparation.Apply(inputIdentifier).ToArray();
            Assert.AreEqual(
                expected: neighbors,
                actual: preparedIdentifier.Where(token => token.Type == Token.IdentifierLiteral).Select(t => (TestNeighor)((t.Data as IdentifierLiteral)?.NeighborSeparator ?? SeparatorNeighbor.None)).ToList());

            var inputOperator = inputTokens.Select(
                t => {
                    var s = t as string;
                    if (s != null) {
                        return new TokenData {
                            Type = Token.OperatorLiteral,
                            Range = new TextFileRange {File = new TextFile {Content = s}, End = new TextPosition {Index = s.Length}},
                            Data = new IdentifierLiteral {Content = s}
                        };
                    }
                    return new TokenData {Type = t};
                });

            var preparedOperator = TokenPreparation.Apply(inputOperator).ToArray();
            Assert.AreEqual(
                expected: neighbors,
                actual: preparedOperator.Where(token => token.Type == Token.OperatorLiteral).Select(t => (TestNeighor)((t.Data as IdentifierLiteral)?.NeighborSeparator ?? SeparatorNeighbor.None)).ToList());
        }
    }
}